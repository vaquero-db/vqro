#include <algorithm>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include "vqro/base/base.h"
#include "vqro/base/worker.h"
#include "vqro/rpc/vqro.pb.h"
#include "vqro/db/db.h"
#include "vqro/db/series.h"
#include "vqro/db/storage_optimizer.h"


DEFINE_int32(read_buffer_size,
             1048576 / vqro::db::datapoint_size, // 43,690 datapoints
             "Number of datapoints to read in one iteration of read work.");
DEFINE_int32(db_worker_threads,
             64,
             "Number of database worker threads.");
DEFINE_int32(flusher_resort_interval,
             5000,
             "How often (milliseconds) the flusher thread will re-sort its "
             "series list.");


namespace vqro {
namespace db {


Database::Database(string dir) {
  if (dir.empty())
    throw std::invalid_argument("No data directory specified");

  root_dir = dir;
  if (root_dir.back() != '/')
    root_dir += "/";

  CreateDirectory(root_dir);

  LOG(INFO) << "initializing search engine";
  search_engine.reset(new SearchEngine(root_dir));

  LOG(INFO) << "initializing storage optimizer";
  storage_optimizer.reset(new StorageOptimizer(this));

  // Create worker threads
  int num_workers = std::max(FLAGS_db_worker_threads, 1);

  LOG(INFO) << "starting " << num_workers << " worker threads";
  while (num_workers--) {
    workers.emplace_back(new WorkerThread());
    workers.back()->Start();
  }
  LOG(INFO) << workers.size() << " worker threads started.";

  // Might make more sense to just have a generic schedule thread and use that plus workers.
  std::thread flusher([&] { FlushWriteBuffers(); });
  flusher.detach();
}


void Database::Write(vqro::rpc::WriteOperation& op)
{
  Series* series = GetSeries(op.series());
  WorkerThread* worker = GetWorker(series);

  worker->Do([&] {
    series->Write(op);
  }).wait();

  if (!series->is_indexed)
    try {
      search_engine->IndexSeries(series);
    } catch (SqliteError& err) {
      LOG(WARNING) << "Failed to index series: " << err.message;
    }
}


void Database::Read(
    const vqro::rpc::Series& series_proto,
    int64_t start_time,
    int64_t end_time,
    int64_t datapoint_limit,
    bool prefer_latest,
    DatapointsCallback callback)
{
  Series* series = GetSeries(series_proto);
  WorkerThread* worker = GetWorker(series);
  std::unique_ptr<Datapoint> read_buffer(new Datapoint[FLAGS_read_buffer_size]); //TODO Arena allocation for read buffers
  vqro::db::ReadOperation read_op(start_time,
                                  end_time,
                                  datapoint_limit,
                                  prefer_latest,
                                  read_buffer.get(),
                                  FLAGS_read_buffer_size);

  // TODO Use two read buffers to keep both threads busy simultaneously
  while (!read_op.Complete()) {
    worker->Do([&] {
      series->Read(read_op);
    }).wait();

    if (!read_op.DatapointsInBuffer())
      break;

    callback(read_op.buffer, read_op.DatapointsInBuffer());
    read_op.ClearBuffer();
  }
}


void Database::SearchSeries(const vqro::rpc::SeriesQuery& query,
                            SearchSeriesResultCallback callback)
{
  try {
    search_engine->SearchSeries(query, callback);
  } catch (SqliteError& err) {
    throw DatabaseError(err.message);
  }
}


void Database::SearchLabels(const vqro::rpc::LabelsQuery& query,
                            SearchLabelsResultCallback callback)
{
  try {
    search_engine->SearchLabels(query, callback);
  } catch (SqliteError& err) {
    throw DatabaseError(err.message);
  }
}


Series* Database::GetSeries(const vqro::rpc::Series& series_proto) {
  // Compute the series' "key" string from its labels
  std::map<string, string> ordered(series_proto.labels().begin(),
                                   series_proto.labels().end());
  string key;
  key.reserve(512); // Append into a once-allocated string for speed
  for (auto it = ordered.begin(); it != ordered.end(); it++) {
    key += it->first;
    key += "=";
    key += it->second;
    key += ";";
  }
  if (key.empty())
    throw InvalidSeriesProto("At least one label is required");

  // Lookup our vqro::db::Series object by key, creating it if it doesn't exist.
  // TODO These objects should expire when unused.
  {
    std::unique_lock<std::mutex> my_lock(series_by_key_mutex);

    auto it = series_by_key.find(key);
    if (it != series_by_key.end())
      return it->second;
    return series_by_key[key] = new Series(this, series_proto, key);
  }
}


WorkerThread* Database::GetWorker(Series* series) {
  return workers[series->keyint % workers.size()];
}


void Database::FlushWriteBuffers() {
  LOG(INFO) << "FlushWriteBuffers thread reporting for duty.";

  //TODO Figure out how to safely delete a Series. This code grabs a pointer
  // while holding the mutex but uses it after releasing.

  vector<Series*> all_series;
  int64_t now = TimeInMillis();
  int64_t resort_deadline = now + FLAGS_flusher_resort_interval;

  while (true) {
    // Prioritize which series should be flushed first.
    all_series.clear();
    all_series.reserve(series_by_key.size());
    {
      std::lock_guard<std::mutex> guard(series_by_key_mutex);
      for (auto it : series_by_key) {
        all_series.push_back(it.second);
      }
    }

    // Sort them to flush the largest buffers first
    std::sort(all_series.begin(), all_series.end(),
        [&] (Series* a, Series* b) -> bool {
          return a->DatapointsBuffered() > b->DatapointsBuffered();
        }
    );

    // Now we flush until its time to re-sort the directories
    int flushed = 0;
    int64_t start = TimeInMillis();

    for (auto series : all_series) {
      if (series->DatapointsBuffered()) {
        series->FlushBufferedDatapoints();
        flushed++;
      } else {
        continue;
      }

      if (flushed % 50 == 0) {
        now = TimeInMillis();
        if (now >= resort_deadline) {
          resort_deadline += FLAGS_flusher_resort_interval;
          break;
        }
      }
    }

    if (flushed) {
      int64_t elapsed = TimeInMillis() - start;
      LOG(INFO) << "Flushed " << flushed << " series in " << elapsed << "ms";
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  } // while (true)
}


} // namespace db
} // namespace vqro
