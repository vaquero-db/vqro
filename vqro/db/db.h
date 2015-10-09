#ifndef VQRO_DB_DB_H
#define VQRO_DB_DB_H

#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "vqro/base/base.h"
#include "vqro/base/worker.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/storage.pb.h"
#include "vqro/db/series.h"
#include "vqro/db/search_engine.h"

#include <gflags/gflags.h>
#include <glog/logging.h>


DECLARE_int32(db_worker_threads);


namespace vqro {
namespace db {


using DatapointsCallback = std::function<void(Datapoint*, size_t)>;


class DatabaseError : public Error {
 public:
  DatabaseError() { Error("Generic DatabaseError"); }
  DatabaseError(string msg) : Error("DatabaseError: " + msg) {}
  DatabaseError(char* msg) : Error(string("DatabaseError: ") + msg) {}
  DatabaseError(const char* msg) : Error(string("DatabaseError: ") + msg) {}
  virtual ~DatabaseError() {}
};


class InvalidSeriesProto : public DatabaseError {
 public:
  InvalidSeriesProto() { DatabaseError("Generic InvalidSeriesProto"); }
  InvalidSeriesProto(string msg) : DatabaseError("InvalidSeriesProto: " + msg) {}
  InvalidSeriesProto(char* msg) : DatabaseError(string("InvalidSeriesProto: ") + msg) {}
  InvalidSeriesProto(const char* msg) : DatabaseError(string("InvalidSeriesProto: ") + msg) {}
  virtual ~InvalidSeriesProto() {}
};


class Database {
 public:
  std::unique_ptr<SearchEngine> search_engine;

  Database(string dir);

  string GetDataDirectory() { return root_dir; }

  void Write(vqro::rpc::WriteOperation& op);

  void Read(const vqro::rpc::Series& series,
            int64_t start_time,
            int64_t end_time,
            int64_t datapoint_limit,
            bool prefer_latest,
            DatapointsCallback callback);

 private:
  string root_dir;
  std::vector<WorkerThread*> workers;
  std::unordered_map<string,Series*> series_by_key {};
  std::mutex series_by_key_mutex;

  Series* GetSeries(const vqro::rpc::Series& series);
  WorkerThread* GetWorker(Series* series);
  void FlushWriteBuffers();
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_DB_H
