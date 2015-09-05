#ifndef VQRO_DB_BUFFER_H
#define VQRO_DB_BUFFER_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

#include <glog/logging.h>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/rpc/vqro.pb.h"
#include "vqro/db/datapoint.h"


namespace vqro {
namespace db {


class WriteBuffer {
 public:
  WriteBuffer();
  ~WriteBuffer() { Clear(); }

  void Write(vqro::rpc::WriteOperation& op);
  bool IsEmpty() { return num_datapoints == 0; }
  bool IsSorted() { return sorted; }
  void Sort() { std::stable_sort(begin(), end()); sorted = true; };

  /// Iterator for Datapoints stored in a WriteBuffer.
  class Iterator : public std::iterator<std::random_access_iterator_tag, Datapoint> {
   public:
    explicit Iterator(WriteBuffer& buf) : buffer(&buf) {}
    Iterator(const Iterator& other) = default; //copy
    Iterator& operator=(const Iterator& other) = default; // assignment
    Iterator& operator=(Iterator&& other) = default; // move assignment
    Iterator& operator+=(const int inc) { *this = *this + inc; return *this; }

    Iterator operator++() { Iterator i = *this; Advance(1); return i; }  //post
    Iterator operator--() { Iterator i = *this; Advance(-1); return i; }  //post
    Iterator operator++(int junk) { Advance(1); return *this; }  //pre
    Iterator operator--(int junk) { Advance(-1); return *this; }  //pre
    Iterator operator+(const int inc) { Iterator i(*this); i.Advance(inc); return i; }
    Iterator operator-(const int inc) { Iterator i(*this); i.Advance(-inc); return i; }
    int operator-(const Iterator& rhs) const { return index - rhs.index; }
    bool operator==(const Iterator& rhs) const { return index == rhs.index; }
    bool operator!=(const Iterator& rhs) const { return index != rhs.index; }
    bool operator<(const Iterator& rhs) const { return index < rhs.index; }
    bool operator>(const Iterator& rhs) const { return index > rhs.index; }
    bool operator<=(const Iterator& rhs) const { return index <= rhs.index; }
    bool operator>=(const Iterator& rhs) const { return index >= rhs.index; }

    void swap(Iterator& other) {
      //if (buffer != other.buffer)
      //  throw logic_error("Cannot swap iterators of different Buffers");
      std::swap(index, other.index);
    }

    Datapoint& operator[](int i) {
      return *(buffer->allocs[i / buffer->datapoints_per_alloc]
               + (i % buffer->datapoints_per_alloc));
    }

    Datapoint& operator*() { return operator[](index); }

    Datapoint* operator->() {
      return (buffer->allocs[index / buffer->datapoints_per_alloc]
              + (index % buffer->datapoints_per_alloc));
    }

    string str() { return std::to_string(index); }

    // Used for writev()
    std::unique_ptr<Iovec[]> GetIOVector(
        size_t& iov_count,
        size_t& writable_datapoints,
        int64_t max_writable_timestamp) const
    {
      iov_count = 0;
      writable_datapoints = 0;
      if (buffer->num_datapoints == 0)
        return std::unique_ptr<Iovec[]>(nullptr);

      if (!buffer->IsSorted())
        buffer->Sort();

      // Limit our Iovec to the last datapoint not exceeding max_writable_timestamp
      Datapoint end_point(max_writable_timestamp, 0.0, 0); 
      auto end_it = std::lower_bound(buffer->begin(), buffer->end(), end_point);
      while (end_it >= buffer->begin() && end_it != buffer->end() &&
             end_it->timestamp > max_writable_timestamp)
        end_it--;  // +duration can still exceed max_writable_timestamp

      // All remaining buffer_it datapoints exceed max_writable_timestamp
      if (end_it < buffer->begin())
        return std::unique_ptr<Iovec[]>(nullptr);

      // Now we can populate our Iovecs' pointers and sizes.
      int alloc_index = index / buffer->datapoints_per_alloc;
      int end_alloc_index = end_it.index / buffer->datapoints_per_alloc;
      iov_count = end_alloc_index - alloc_index + 1;
      std::unique_ptr<Iovec[]> iov(new Iovec[iov_count]);

      int alloc_offset;
      int i = index;
      while (i <= end_it.index) {
        alloc_index = i / buffer->datapoints_per_alloc;
        alloc_offset = i % buffer->datapoints_per_alloc;

        iov[alloc_index].iov_base = buffer->allocs[alloc_index] + alloc_offset;
        i += iov[alloc_index].iov_len = std::min(
            buffer->datapoints_per_alloc - alloc_offset, // already a size
            end_it.index - i + 1); // +1 converts from difference of indices to a size
        iov[alloc_index].iov_len *= vqro::datapoint_size; // convert from datapoints to bytes
      }
      writable_datapoints = i - index;
      return iov;
    }

   private:
    WriteBuffer* buffer;
    int index = 0;  // Our index into the buffer for the current Datapoint

    void Advance(int increment) {
      index = std::max(0UL, std::min(static_cast<size_t>(index + increment),
                                     buffer->num_datapoints));
    }
  };
  friend class Iterator;

  Iterator begin() {
    return Iterator(*this);
  }

  Iterator end() {
    return Iterator(*this) + num_datapoints;
  }

  inline size_t Size() { return num_datapoints; }

  // Empties the buffer and resets state.
  void Clear() {
    for (auto alloc : allocs)
      free(alloc);
    allocs.clear();
    num_datapoints = 0;
    sorted = true;
  }

 private:
  size_t alloc_size;  // Size of each allocation in bytes
  std::vector<Datapoint*> allocs;  // The allocations where our data is stored.
  int datapoints_per_alloc;  // Number of datapoints that can fit in one of our allocs
  size_t num_datapoints = 0;  // Number of datapoints stored in our allocs
  bool sorted = true;  // Can become false as datapoints get written

  Datapoint* GetNextDatapoint();
};


inline void swap(WriteBuffer::Iterator& a,
                 WriteBuffer::Iterator& b)
{
  a.swap(b);
}


} // namespace db
} //namespace vqro

#endif // VQRO_DB_BUFFER_H
