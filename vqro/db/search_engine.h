#ifndef VQRO_DB_SEARCH_ENGINE_H
#define VQRO_DB_SEARCH_ENGINE_H

#include <functional>
#include <unordered_set>

#include <sqlite3.h>

#include "vqro/base/base.h"
#include "vqro/rpc/search.pb.h"
#include "vqro/db/series.h"
#include "vqro/db/sql_statement.h"


namespace vqro {
namespace db {


using SearchSeriesResultsCallback = std::function<void(vqro::rpc::SearchSeriesResults&)>;
using SearchLabelsResultsCallback = std::function<void(vqro::rpc::SearchLabelsResults&)>;


class SearchEngine {
 public:
  SearchEngine(string db_dir);

  void IndexSeries(Series* series);

  void SearchSeries(const vqro::rpc::SeriesQuery& query,
                    SearchSeriesResultsCallback callback);

  void SearchLabels(const vqro::rpc::LabelsQuery& query,
                    SearchLabelsResultsCallback callback);

 private:
  sqlite3* sqlite_db = NULL;
  std::unordered_set<string> all_labels;

  void MaybeThrowSqliteError(int return_code, string message);
  void IndexLabel(int64_t series_id, string name, string value);
  SqlStatement Prepare(string sql);
};


} // namespace db
} // namespace vqro

#endif
