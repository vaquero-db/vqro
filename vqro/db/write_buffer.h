#ifndef VQRO_DB_BUFFER_H
#define VQRO_DB_BUFFER_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/rpc/storage.pb.h"
#include "vqro/db/datapoint.h"
#include "vqro/db/datapoint_buffer.h"


namespace vqro {
namespace db {


class WriteBuffer: public DatapointBuffer {
 private:
  size_t alloc_size;  // Size of each allocation in bytes
  std::vector<Datapoint*> allocs;  // The allocations where our data is stored.
  size_t datapoints_per_alloc;  // Number of datapoints that can fit in one of our allocs
  size_t num_datapoints = 0;  // Number of datapoints stored in our allocs
  bool sorted = true;  // Can become false as datapoints get written


  class WriteIterImpl: public IteratorImpl {
   private:
    WriteBuffer* const buf;

   public:
    explicit WriteIterImpl(WriteBuffer* b) : buf(b) {}

    Datapoint& ValueAt(size_t i) const override {
      return *(buf->allocs[i / buf->datapoints_per_alloc]
               + (i % buf->datapoints_per_alloc));
    }

    IteratorImpl* Clone() const override {
      return static_cast<IteratorImpl*>(new WriteIterImpl(buf));
    }

    std::unique_ptr<Iovec[]> GetIOVector(
        const size_t pos,
        size_t& iov_count,
        size_t& writable_datapoints,
        size_t max_writable_datapoints,
        const int64_t max_writable_timestamp) const override
    {
      iov_count = 0;
      writable_datapoints = 0;
      if (buf->num_datapoints == 0)
        return nullptr;

      if (!buf->IsSorted())
        buf->Sort();

      // Limit our Iovec to the last datapoint not exceeding max_writable_timestamp
      Datapoint end_point(max_writable_timestamp, 0.0, 0); 
      auto end_it = std::upper_bound(buf->begin(), buf->end(), end_point);

      // All remaining datapoints exceed max_writable_timestamp
      if (end_it.Pos() <= pos)
        return nullptr;

      // Figure out how many allocs/iovecs we will need
      int alloc_index = pos / buf->datapoints_per_alloc;
      int last_alloc_index = (end_it.Pos() - 1) / buf->datapoints_per_alloc;
      int max_iovecs = max_writable_datapoints / buf->datapoints_per_alloc;
      if (max_writable_datapoints % buf->datapoints_per_alloc) max_iovecs++;
      iov_count = std::min(max_iovecs, last_alloc_index - alloc_index + 1);
      std::unique_ptr<Iovec[]> iov(new Iovec[iov_count]);

      // Now we can populate our Iovecs' pointers and sizes. We do this by
      // walking a position 'i' forward from our current pos to end_it.Pos().
      // Name clarifications:
      //   'i' specifies where in the buffer we are looking at
      //   'alloc_index' specifies which alloc 'i' is referring to
      //   'alloc_offset' specifies where in that alloc 'i' is referring to
      //   'alloc_usable' is how many datapoints will be written from the alloc
      // Also note that we decrease max_writable_datapoints as we go to know
      // when we've hit that limit.
      size_t alloc_offset;
      size_t alloc_usable;
      size_t i = pos;
      while (i < end_it.Pos() && max_writable_datapoints > 0) {
        alloc_index = i / buf->datapoints_per_alloc;
        alloc_offset = i % buf->datapoints_per_alloc;

        // Note that alloc_usable will generally be the whole alloc except when
        // we hit the end of an alloc we were in the middle of, or if we run
        // out of datapoints in the buffer, or we hit max_writable_datapoints.
        alloc_usable = std::min({
            end_it.Pos() - i,                          // til buffer end
            buf->datapoints_per_alloc - alloc_offset,  // til alloc end
            max_writable_datapoints});                 // til our limit

        iov[alloc_index].iov_base = buf->allocs[alloc_index] + alloc_offset;
        iov[alloc_index].iov_len = alloc_usable * datapoint_size;
        i += alloc_usable;
        max_writable_datapoints -= alloc_usable;
      }
      writable_datapoints = i - pos;
      return iov;
    }
  };
  friend class WriteIterImpl;

 public:
  WriteBuffer();
  ~WriteBuffer() { Clear(); }

  // DatapointsBuffer API
  size_t Size() const { return num_datapoints; }
  Iterator begin() { return Iterator(new WriteIterImpl(this), 0); }
  Iterator end() { return Iterator(new WriteIterImpl(this), num_datapoints); }

  // WriteBuffer-specific API
  void Append(vqro::rpc::WriteOperation& op);
  bool IsEmpty() { return num_datapoints == 0; }
  bool IsSorted() { return sorted; }
  void Sort() { std::stable_sort(begin(), end()); sorted = true; };

  void Clear() {
    for (auto alloc : allocs)
      free(alloc);
    allocs.clear();
    num_datapoints = 0;
    sorted = true;
  }

 private:
  Datapoint* GetNextDatapoint();
};


} // namespace db
} //namespace vqro

#endif // VQRO_DB_BUFFER_H
