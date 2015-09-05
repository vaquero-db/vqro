#ifndef VQRO_DB_SPARSE_FILE_H
#define VQRO_DB_SPARSE_FILE_H

#include <cstdint>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/db/datapoint.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"


namespace vqro {
namespace db {


class DatapointDirectory;


class SparseFile: public DatapointFile {
 public:
  SparseFile() = default;
  SparseFile(DatapointDirectory* _dir, int64_t _min, int64_t _max) :
    DatapointFile(_dir, _min, _max) {}

  static std::unique_ptr<DatapointFile> FromFilename(
      DatapointDirectory* dir,
      char* filename);

  string GetPath();
  void Read(ReadOperation& read_op);
  size_t Write(const WriteOperation& write_op);
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_SPARSE_FILE_H
