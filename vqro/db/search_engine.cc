#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <re2/re2.h>

#include "vqro/db/search_engine.h"
#include "vqro/db/series.h"


DEFINE_string(search_db_file, "search.db", "Filename of sqlite3 database "
              "for search engine.");
DEFINE_string(sqlite_pragma_synchronous, "NORMAL",
              "Value to use for 'PRAGMA synchronous'");
DEFINE_string(sqlite_pragma_locking_mode, "EXCLUSIVE",
              "Value to use for 'PRAGMA locking_mode'");
DEFINE_string(sqlite_pragma_journal_mode,"WAL",
              "Value to use for 'PRAGMA journal_mode'");
DEFINE_int32(search_results_batch_size, 1024, "Maximum number of results "
             "contained in each search result protobuf.");


namespace vqro {
namespace db {


void sqlite_regexp_re2(sqlite3_context* context, int argc, sqlite3_value** argv) {
  int matched = 0;
  if (argc == 2) {
    string pattern = reinterpret_cast<const char*>(sqlite3_value_text(argv[0]));
    string text = reinterpret_cast<const char*>(sqlite3_value_text(argv[1]));
    matched = static_cast<int>(RE2::FullMatch(text, pattern));
  }
  sqlite3_result_int(context, matched);
}


string SqlQuoteIdentifier(string raw) {
  constexpr int buf_size = 4096;
  char buf[buf_size];
  sqlite3_snprintf(buf_size, buf, "\"%w\"", raw.c_str());
  return string(buf);
}


SearchEngine::SearchEngine(string db_dir) {
  int ret;

  // Open/Create the sqlite db file
  string filename = db_dir + FLAGS_search_db_file;
  LOG(INFO) << "Opening sqlite database: " << filename;

  ret = sqlite3_open(filename.c_str(), &sqlite_db);
  MaybeThrowSqliteError(ret, "sqlite3_open() failed");

  string pragmas = (
    "PRAGMA locking_mode=" + FLAGS_sqlite_pragma_locking_mode + ";\n" +
    "PRAGMA journal_mode=" + FLAGS_sqlite_pragma_journal_mode + ";\n" +
    "PRAGMA synchronous="  + FLAGS_sqlite_pragma_synchronous  + ";\n");
  LOG(INFO) << "Setting sqlite pragmas\n" << pragmas;
  ret = sqlite3_exec(sqlite_db, pragmas.c_str(), NULL, NULL, NULL);
  MaybeThrowSqliteError(ret, "Failed to set pragmas");

  //TODO throw an error if the pragmas do not have the prescribed values

  // Register our regexp() function
  ret = sqlite3_create_function_v2(
    sqlite_db,                           // db cursor
    "regexp",                            // function name
    2,                                   // num args
    SQLITE_UTF8 | SQLITE_DETERMINISTIC,  // eTextRep
    NULL,                                // void* pApp
    &sqlite_regexp_re2,                  // xFunc
    NULL,                                // xStep
    NULL,                                // xFinal
    NULL                                 // xDestroy
  );
  MaybeThrowSqliteError(ret, "Failed to create REGEXP sqlite function");
  LOG(INFO) << "sqlite REGEXP function registered";

  // Initialize internal tables
  string init_sql = R"(
    CREATE TABLE IF NOT EXISTS "vqro:series" (
        series_id INTEGER PRIMARY KEY NOT NULL,
        key TEXT UNIQUE NOT NULL,
        protobuf BLOB NOT NULL);
  )";
  ret = sqlite3_exec(sqlite_db, init_sql.c_str(), NULL, NULL, NULL);
  MaybeThrowSqliteError(ret, "Failed to sqlite3_exec() table initialization sql");

  // Scan for all existing label tables
  string scan_sql = R"(
    SELECT name
    FROM SQLITE_MASTER
    WHERE type='table' AND name NOT LIKE 'vqro:%';
  )";
  auto callback = [] (void* all_labels, int cols, char** values, char** names) -> int {
    static_cast<std::unordered_set<string>*>(all_labels)->insert(values[0]);
    return 0;
  };
  ret = sqlite3_exec(sqlite_db, scan_sql.c_str(), callback, &all_labels, NULL);
  MaybeThrowSqliteError(ret, "Failed to sqlite3_exec() table scan sql");
}


SqlStatement SearchEngine::Prepare(string sql) {
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(sqlite_db,
                         sql.c_str(),
                         -1,
                         &stmt,
                         NULL) != SQLITE_OK)
    throw SqliteError(string("Prepare() failed: ") + sqlite3_errmsg(sqlite_db));
  return SqlStatement(sqlite_db, stmt); // move semantics, no destuctor call
}


void SearchEngine::MaybeThrowSqliteError(int return_code, string message) {
  if (return_code != SQLITE_OK)
    throw SqliteError(message + ": " + sqlite3_errmsg(sqlite_db));
}


void SearchEngine::IndexSeries(Series* series) {
  //TODO Figure out how to hold onto these prepared statements thread-safely.
  SqlStatement series_insert = Prepare(
      R"(INSERT OR IGNORE INTO "vqro:series" (key, protobuf) VALUES (?, ?);)");
  SqlStatement begin_transaction = Prepare("BEGIN TRANSACTION");
  SqlStatement end_transaction = Prepare("END TRANSACTION");

  string proto_str;
  if (!series->proto.SerializeToString(&proto_str)) {
    LOG(ERROR) << "Failed to serialize series protobuf!";
    throw SqliteError("Failed to serialize series protobuf!");
  }

  series_insert.BindText(1, series->keystr);
  series_insert.BindText(2, proto_str);
  series_insert.Execute();

  // Now we create the series' label tables and insert our label values into them
  int64_t series_id = sqlite3_last_insert_rowid(sqlite_db);

  if (series_id) { // No insert happened if the row already existed
    LOG(INFO) << "Indexing series: " << series->keystr;
    begin_transaction.Execute();
    for (auto it : series->proto.labels()) {
      IndexLabel(series_id, it.first, it.second);
    }
    end_transaction.Execute();
  }
  series->is_indexed = true;
}


void SearchEngine::IndexLabel(int64_t series_id, string name, string value) {
  LOG(INFO) << "IndexLabel() series_id=" << series_id << " " << name << "=" << value;
  string table_name = SqlQuoteIdentifier(name);

  // Create the label table
  string create_sql = "CREATE TABLE IF NOT EXISTS " + table_name +
      " (series_id INTEGER PRIMARY KEY NOT NULL, value TEXT NOT NULL)";

  int ret = sqlite3_exec(sqlite_db, create_sql.c_str(), NULL, NULL, NULL);
  MaybeThrowSqliteError(ret, "Failed to sqlite3_exec() label table creation sql");

  // Insert the label value for this series into the label table
  SqlStatement insert = Prepare(
      "INSERT OR IGNORE INTO " + table_name + " (series_id, value) VALUES (?, ?);");
  insert.BindInt64(1, series_id);
  insert.BindText(2, value);
  insert.Execute();
}


void SearchEngine::SearchSeries(const vqro::rpc::SeriesQuery& query,
                                SearchSeriesResultCallback callback)
{
  std::vector<string> parameters;
  string sql;

  if (query.constraints_size() == 0)
    return;

  sql.reserve(1024);
  sql += R"(SELECT protobuf FROM "vqro:series" )";
  for (auto it : query.constraints()) {
    string table = SqlQuoteIdentifier(it.label_name());
    sql += "INNER JOIN " + table
        + R"( ON "vqro:series".series_id = )" + table + ".series_id ";
  }
  sql += "WHERE ";

  int constraints_left = query.constraints_size();
  for (auto it : query.constraints()) {
    string table = SqlQuoteIdentifier(it.label_name());
    sql += table + ".value "
        + ((it.constraint_op() == vqro::rpc::LabelConstraint::EQUALS) ? "=" : "REGEXP")
        + " ? ";
    parameters.push_back(it.operand());

    if (--constraints_left)
      sql += "AND ";
  }

  if (query.result_limit()) {
    sql += "LIMIT " + to_string(query.result_limit());
    if (query.result_offset())
      sql += " OFFSET " + to_string(query.result_offset());
  }
  sql += ";";

  // create prepared statement and bind parameters
  VLOG(2) << "SearchSeries() sql: " << sql;
  SqlStatement select = Prepare(sql);

  for (unsigned int i = 0; i < parameters.size(); i++) {
    select.BindText(i + 1, parameters[i]);
  }

  // Read the rows into result protos we can feed to our callback function
  int ret;
  int row_count = 0;
  vqro::rpc::SearchSeriesResult result;
  while (true) {
    ret = sqlite3_step(select.stmt);

    if (ret != SQLITE_ROW)
      break;

    const char* protobuf = static_cast<const char*>(sqlite3_column_blob(select.stmt, 0));
    int len = sqlite3_column_bytes(select.stmt, 0);
    bool parsed_ok = result.add_matching_series()->ParseFromArray(protobuf, len);
    row_count++;

    if (!parsed_ok) {
      LOG(ERROR) << "Error: vqro:series row contains invalid protobuf";
      result.mutable_matching_series()->RemoveLast();
      // I *could* callback(result) sans labels but pass an error via the StatusMessage
      continue;
    }

    // Batch results for callbacks
    if (result.matching_series_size() >= FLAGS_search_results_batch_size) {
      callback(result);
      result.Clear();
    }
  }

  // Handle the final batch that didn't exceed the limit
  if (result.matching_series_size())
    callback(result);

  // ret != SQLITE_ROW so we're done processing rows
  if (ret == SQLITE_DONE) {
    VLOG(1) << "Query successfully matched " << row_count << " rows";
    return;
  }
  throw SqliteError(string("sqlite3_step() error: ") + sqlite3_errmsg(sqlite_db));
}


void SearchEngine::SearchLabels(const vqro::rpc::LabelsQuery& query,
                                SearchLabelsResultCallback callback)
{
  if (query.regex().empty())
    return;

  string sql = R"(
    SELECT name
    FROM SQLITE_MASTER
    WHERE type='table' AND
    name NOT LIKE 'vqro:%' AND
    name REGEXP ?
  )";

  if (query.result_limit()) {
    sql += " LIMIT " + to_string(query.result_limit());
    if (query.result_offset())
      sql += " OFFSET " + to_string(query.result_offset());
  }
  sql += ";";

  VLOG(2) << "SearchLabels() sql: " << sql;
  SqlStatement select = Prepare(sql);
  select.BindText(1, query.regex());

  int ret;
  int row_count = 0;
  vqro::rpc::SearchLabelsResult result;
  while (true) {
    ret = sqlite3_step(select.stmt);

    if (ret != SQLITE_ROW)
      break;

    const char* str = static_cast<const char*>(sqlite3_column_blob(select.stmt, 0));
    int len = sqlite3_column_bytes(select.stmt, 0);
    result.add_labels(str, len);
    row_count++;

    // Batch results for callbacks
    if (result.labels_size() >= FLAGS_search_results_batch_size) {
      callback(result);
      result.Clear();
    }
  }

  // Handle the final batch that didn't exceed the limit
  if (result.labels_size())
    callback(result);

  // ret != SQLITE_ROW so we're done processing rows
  if (ret == SQLITE_DONE) {
    VLOG(1) << "Query successfully matched " << row_count << " rows";
    return;
  }
  throw SqliteError(string("sqlite3_step() error: ") + sqlite3_errmsg(sqlite_db));
}


} // namespace db
} // namespace vqro
