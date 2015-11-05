#ifndef VQRO_DB_SPARSE_FILE_H
#define VQRO_DB_SPARSE_FILE_H

#include <cstdint>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/db/datapoint.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"


DECLARE_int64(sparse_file_max_size);


namespace vqro {
namespace db {


class DatapointDirectory;

// Filename suffix for optimized sparse files.
constexpr const char* SPARSE_OPT_SUFFIX = ".opt";
constexpr int SPARSE_OPT_SUFFIX_LEN = sizeof(SPARSE_OPT_SUFFIX);


class SparseFile: public DatapointFile {
 public:
  SparseFile() = default;
  SparseFile(DatapointDirectory* _dir, int64_t _min, int64_t _max, bool opt=false) :
    DatapointFile(_dir, _min, _max), optimized(opt) {}

  static std::unique_ptr<DatapointFile> FromFilename(
      DatapointDirectory* dir,
      char* filename);

  string GetPath() const;
  void Read(ReadOperation& read_op) const;
  size_t Write(const WriteOperation& write_op);
  size_t RemainingWritableDatapoints() const;

 private:
  bool optimized = false;

  void FileIsTooBig();
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_SPARSE_FILE_H
