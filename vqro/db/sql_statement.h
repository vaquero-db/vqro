#ifndef VQRO_DB_SQL_STATEMENT_H
#define VQRO_DB_SQL_STATEMENT_H

#include <algorithm>
#include <string>
#include <sqlite3.h>
#include <glog/logging.h>


using std::string;


namespace vqro {
namespace db {


class SqliteError : public Error {
 public:
  SqliteError() { Error("Generic SqliteError"); }
  SqliteError(string msg) : Error("SqliteError: " + msg) {}
  SqliteError(char* msg) : Error(string("SqliteError: ") + msg) {}
  SqliteError(const char* msg) : Error(string("SqliteError: ") + msg) {}
  virtual ~SqliteError() {}
};


class SqlStatement {
 public:
  sqlite3* db = NULL;
  sqlite3_stmt* stmt = NULL;

  SqlStatement() = default;
  SqlStatement(sqlite3* sql_db, sqlite3_stmt* sql_stmt) :
      db(sql_db),
      stmt(sql_stmt) {}

  // disable copy construction and copy assignment
  SqlStatement(const SqlStatement& other) = delete;
  SqlStatement& operator=(const SqlStatement& other) = delete;

  SqlStatement(SqlStatement&& other) { // move constructor
    std::swap(stmt, other.stmt);
    std::swap(db, other.db);
  }

  SqlStatement& operator=(SqlStatement&& other) { // move assignment
    std::swap(stmt, other.stmt);
    std::swap(db, other.db);
    return *this;
  }

  ~SqlStatement() {
    sqlite3_finalize(stmt);
  }

  void BindText(int param_num, string val) {
    if (sqlite3_bind_text(stmt,
                          param_num,
                          val.c_str(),
                          val.size(),
                          SQLITE_STATIC) != SQLITE_OK)
      Throw("sqlite3_bind_text() failed");
  }

  void BindInt64(int param_num, int64_t val) {
    if (sqlite3_bind_int64(stmt,
                           param_num,
                           val) != SQLITE_OK)
      Throw("sqlite3_bind_int64() failed");
  }

  void Execute() {
    sqlite3_reset(stmt);
    if (sqlite3_step(stmt) != SQLITE_DONE)
      Throw("sqlite3_step() != SQLITE_DONE");
  }

 private:
  void Throw(string msg) {
    throw SqliteError(msg + ": " + sqlite3_errmsg(db) +
                      ", query=" + sqlite3_sql(stmt));
  }
};


} // namespace db
} // namespace vqro


#endif // VQRO_DB_SQL_STATEMENT_H
