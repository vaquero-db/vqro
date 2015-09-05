#ifndef VQRO_DB_DENSE_FILE_H
#define VQRO_DB_DENSE_FILE_H

#include <cstdint>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/db/datapoint.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"

#define DENSE_DATAPOINT_SIZE sizeof(double)


namespace vqro {
namespace db {


class DatapointDirectory;


class DenseFile: public DatapointFile {
 public:
  int64_t duration;

  DenseFile() = default;
  DenseFile(DatapointDirectory* _dir, int64_t start_time, int64_t dur) :
    DatapointFile(_dir, start_time, start_time),
    duration(dur) {
      max_timestamp = start_time + GetFileSize(GetPath()) / DENSE_DATAPOINT_SIZE * dur;
    }

  static std::unique_ptr<DatapointFile> FromFilename(
      DatapointDirectory* dir,
      char* filename);

  string GetPath();
  void Read(ReadOperation& read_op);
  size_t Write(const WriteOperation& write_op);
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_DENSE_FILE_H
