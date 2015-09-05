#ifndef VQRO_DB_WRITE_OP_H
#define VQRO_DB_WRITE_OP_H

#include <cstdint>
#include <memory>
#include "vqro/base/base.h"
#include "vqro/db/write_buffer.h"


namespace vqro {
namespace db {


struct WriteOperation {
  WriteOperation(WriteBuffer* buf) :
    buffer(buf),
    cursor(buf->begin()),
    max_writable_timestamp(INT64_MAX) {}

  WriteOperation(const WriteOperation& other) = delete; // no copying
  WriteOperation& operator=(const WriteOperation& other) = delete; // no assignment

  WriteBuffer* buffer;
  WriteBuffer::Iterator cursor;
  int64_t max_writable_timestamp;

  bool Complete() { return cursor == buffer->end(); }

  size_t WritableDatapoints() const {
    auto it = cursor;
    while (it != buffer->end() && it->timestamp < max_writable_timestamp)
      it++;
    return std::distance(cursor, it);
  }

  std::unique_ptr<Iovec[]> GetIOVector(size_t& iov_len,
                                       size_t& writable_datapoints) const
  {
    return cursor.GetIOVector(iov_len,
                              writable_datapoints,
                              max_writable_timestamp);
  }
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_WRITE_OP_H
