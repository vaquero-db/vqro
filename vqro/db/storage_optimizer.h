#ifndef VQRO_DB_STORAGE_OPTIMIZER_H
#define VQRO_DB_STORAGE_OPTIMIZER_H

#include <memory>
#include "vqro/base/base.h"
#include "vqro/db/sparse_file.h"


namespace vqro {
namespace db {


class Database;

class StorageOptimizer {
 public:
  Database* const db;

  StorageOptimizer(Database* _db) : db(_db) {}

  void SparseFileTooBig(SparseFile* sparse_file);

 private:
  void HandleSparseFileTooBig(SparseFile& sparse_file);
  bool IsDense(Datapoint* buf, size_t len);
  bool IsConstant(Datapoint* buf, size_t len);

  void ConvertSparseToDense(const SparseFile& sparse_file,
                            Datapoint* buf,
                            size_t len);
  void ConvertSparseToConstant(const SparseFile& sparse_file,
                               Datapoint* buf,
                               size_t len);
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_STORAGE_OPTIMIZER_H
