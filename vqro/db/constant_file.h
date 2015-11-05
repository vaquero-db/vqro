#ifndef VQRO_DB_CONSTANT_FILE_H
#define VQRO_DB_CONSTANT_FILE_H

#include <cstdint>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"

namespace vqro {
namespace db {


class DatapointDirectory;


class ConstantFile: public DatapointFile {
 public:
  const int64_t duration;
  int64_t count;
  const double value;

  ConstantFile() = default;
  ConstantFile(DatapointDirectory* _dir,
               int64_t start_time,
               int64_t dur,
               int64_t cnt,
               double val) :
      DatapointFile(_dir, start_time, start_time + cnt * dur),
      duration(dur),
      count(cnt),
      value(val) {}

  static std::unique_ptr<DatapointFile> FromFilename(
      DatapointDirectory* dir,
      char* filename);

  string GetPath() const;
  void Read(ReadOperation& read_op) const;
  size_t Write(const WriteOperation& write_op);
  size_t RemainingWritableDatapoints() const { return -1; }
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_CONSTANT_FILE_H
