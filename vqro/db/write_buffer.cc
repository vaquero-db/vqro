#include <algorithm>
#include <cmath>
#include <exception>
#include <stdlib.h>

#include "vqro/base/base.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/storage.pb.h"
#include "vqro/db/write_buffer.h"
#include "vqro/db/datapoint.h"


DEFINE_int32(write_buffer_pages_per_alloc,
             1,
             "Datapoint write buffers grow by allocating this many "
             "pages each time the buffer fills up.");


namespace vqro {
namespace db {


WriteBuffer::WriteBuffer() {
  alloc_size = pagesize * FLAGS_write_buffer_pages_per_alloc;
  datapoints_per_alloc = alloc_size / datapoint_size;
}


void WriteBuffer::Append(vqro::rpc::WriteOperation& op) {
  Datapoint* next;
  Datapoint* previous = nullptr;

  for (int i = 0; i < op.datapoints_size(); i++) {
    const vqro::rpc::Datapoint& op_datapoint = op.datapoints(i);

    if (std::isnan(op_datapoint.value()))  // NANs not allowed
      continue;

    next = GetNextDatapoint();
    next->timestamp = op_datapoint.timestamp();
    next->value = op_datapoint.value();
    next->duration = op_datapoint.duration();
    num_datapoints++;

    if (sorted && previous != nullptr &&
        next->timestamp < previous->timestamp)
      sorted = false;

    previous = next;
  }
}


Datapoint* WriteBuffer::GetNextDatapoint() {
  size_t alloc_index = num_datapoints / datapoints_per_alloc;
  size_t alloc_offset = num_datapoints % datapoints_per_alloc;

  if (alloc_index == allocs.size()) {
    void* ptr = aligned_alloc(pagesize, alloc_size);
    if (ptr == NULL)
      throw std::bad_alloc();

    allocs.push_back( static_cast<Datapoint*>(ptr) );
  }
  return allocs[alloc_index] + alloc_offset;
}


} // namespace db
} // namespace vqro
