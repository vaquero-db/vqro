#ifndef VQRO_DB_SERIES_H
#define VQRO_DB_SERIES_H

#include <functional>

#include "vqro/base/base.h"
#include "vqro/rpc/vqro.pb.h"
#include "vqro/db/datapoint_directory.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"
#include "vqro/db/write_buffer.h"


namespace vqro {
namespace db {

class Database;


inline static size_t ComputeHash(const string& s) {
  std::hash<string> h;
  return h(s);
}


class Series {
 public:
  Database* const db;
  std::unique_ptr<WriteBuffer> write_buffer;
  const vqro::rpc::Series proto;
  const string keystr;
  const size_t keyint;
  bool is_indexed = false;

  Series(Database* d, const vqro::rpc::Series& pb, string key) :
    db(d),
    write_buffer(new WriteBuffer()),
    proto(pb),
    keystr(key),
    keyint(ComputeHash(key)) { Init(); }

  void Write(vqro::rpc::WriteOperation& op);
  void Read(ReadOperation& op);
  size_t DatapointsBuffered();
  void FlushBufferedDatapoints();

 private:
  void Init();

  std::unique_ptr<DatapointDirectory> data_dir;
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_SERIES_H
