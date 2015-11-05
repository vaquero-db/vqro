#ifndef VQRO_DB_DATAPOINT_DIRECTORY_H
#define VQRO_DB_DATAPOINT_DIRECTORY_H

#include <exception>
#include <vector>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"
#include "vqro/db/datapoint.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/storage_optimizer.h"

namespace vqro {
namespace db {


class Series;


class DatapointDirectory {
  friend class StorageOptimizer;

 public:
  Series* const series;
  string path;

  DatapointDirectory(Series* s, string p) :
    series(s),
    path(p)
  {
    while (!path.empty() && path.back() == '/')
      path = path.substr(0, path.length() - 1);

    if (path.empty())
      throw std::logic_error("DatapointDirectory path cannot be empty..");
  }

  //disable copy & assign
  DatapointDirectory(const DatapointDirectory& other) = delete;
  DatapointDirectory& operator=(const DatapointDirectory& other) = delete;

  void Write(WriteOperation& wrote_op);
  void Read(ReadOperation& read_op);

 private:
  bool filenames_read = false;
  vector<std::unique_ptr<DatapointFile>> datapoint_files {};

  void ReadFilenames();

  vector<std::unique_ptr<DatapointFile>>::iterator FindFirstPotentialFile(
      int64_t timestamp);
};


inline bool operator<(const std::unique_ptr<DatapointFile>& lhs,
                      const std::unique_ptr<DatapointFile>& rhs) {
  return lhs->min_timestamp < rhs->min_timestamp;
}


} // namespace db
} // namespace vqro

#endif // VQRO_DB_DATAPOINT_DIRECTORY_H
