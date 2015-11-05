#ifndef VQRO_DB_RAW_BUFFER_H
#define VQRO_DB_RAW_BUFFER_H
#include "vqro/db/datapoint_buffer.h"


namespace vqro {
namespace db {


class RawBuffer: public DatapointBuffer {
 private:
  Datapoint* const buf;
  const size_t len;

  class RawIterImpl: public IteratorImpl {
   public:
    explicit RawIterImpl(Datapoint* b, size_t l) :
        buf(b),
        len(l) {}

    Datapoint& ValueAt(size_t i) const override { return buf[i]; }

    IteratorImpl* Clone() const override {
      return static_cast<IteratorImpl*>(new RawIterImpl(buf, len));
    }

    std::unique_ptr<Iovec[]> GetIOVector(
        const size_t pos,
        size_t& iov_count,
        size_t& writable_datapoints,
        size_t max_writable_datapoints,
        const int64_t max_writable_timestamp) const override
    {
      iov_count = 1;
      writable_datapoints = std::min((len > pos) ? (len - pos) : 0,
                                     max_writable_datapoints);
      while (writable_datapoints > 0 &&
             buf[pos + writable_datapoints - 1].timestamp > max_writable_timestamp)
        writable_datapoints--;

      if (!writable_datapoints) {
        iov_count = 0;
        return std::unique_ptr<Iovec[]>(nullptr);
      }

      std::unique_ptr<Iovec[]> iov(new Iovec[iov_count]);
      iov[0].iov_base = buf + pos;
      iov[0].iov_len = writable_datapoints;
      return iov;
    }

   private:
    Datapoint* buf;
    size_t len;
  };

 public:
  RawBuffer(Datapoint* b, size_t l) : buf(b), len(l) {}
  RawBuffer(const RawBuffer& other) = delete; // no copying
  RawBuffer& operator=(const RawBuffer& other) = delete; // no assignment

  size_t Size() const override { return len; }
  Iterator begin() { return Iterator(new RawIterImpl(buf, len), 0); }
  Iterator end() { return Iterator(new RawIterImpl(buf, len), len); }
};


}  // namespace db
}  // namespace vqro

#endif  // VQRO_DB_RAW_BUFFER_H
