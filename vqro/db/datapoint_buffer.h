#ifndef VQRO_DB_DATAPOINT_BUFFER_H
#define VQRO_DB_DATAPOINT_BUFFER_H


namespace vqro {
namespace db {


class DatapointBuffer {
 protected:
  class IteratorImpl {
   public:
    virtual ~IteratorImpl() {}

    virtual Datapoint& ValueAt(size_t i) const = 0;
    virtual IteratorImpl* Clone() const = 0;
    virtual std::unique_ptr<Iovec[]> GetIOVector(
        const size_t pos,
        size_t& iov_count,
        size_t& writable_datapoints,
        size_t max_writable_datapoints,
        const int64_t max_writable_timestamp) const = 0;
  };

 public:
  class Iterator: public std::iterator<std::random_access_iterator_tag, Datapoint> {
   public:
    explicit Iterator(IteratorImpl* i, size_t p) : impl(i), pos(p) {}

    Iterator(const Iterator& other) :  // copy construct
        impl(other.impl->Clone()),
        pos(other.pos) {}

    Iterator(Iterator&& other) :  // move construct
        impl(std::move(other.impl)),
        pos(other.pos) {}

    Iterator& operator=(const Iterator& other) {  // copy assign
      impl.reset(other.impl->Clone());
      pos = other.pos;
      return *this;
    }

    Iterator& operator=(Iterator&& other) {  // move assign
      impl.swap(other.impl);
      pos = other.pos;
      return *this;
    }

    Iterator& operator+=(const long inc) { pos += inc; return *this; }
    Iterator operator++() { Iterator i = *this; pos++; return i; }  //post
    Iterator operator--() { Iterator i = *this; pos--; return i; }  //post
    Iterator operator++(int) { pos++; return *this; }  //pre
    Iterator operator--(int) { pos--; return *this; }  //pre
    Iterator operator+(const long inc) const { Iterator i(*this); i.pos += inc; return i; }
    Iterator operator-(const long inc) const { Iterator i(*this); i.pos -= inc; return i; }
    long operator-(const Iterator& rhs) const { return pos - rhs.pos; }
    bool operator==(const Iterator& rhs) const { return pos == rhs.pos; }
    bool operator!=(const Iterator& rhs) const { return pos != rhs.pos; }
    bool operator<(const Iterator& rhs) const { return pos < rhs.pos; }
    bool operator>(const Iterator& rhs) const { return pos > rhs.pos; }
    bool operator<=(const Iterator& rhs) const { return pos <= rhs.pos; }
    bool operator>=(const Iterator& rhs) const { return pos >= rhs.pos; }
    Datapoint& operator[](long i) { return impl->ValueAt(i); }
    Datapoint& operator*() { return impl->ValueAt(pos); }
    Datapoint* operator->() { return &operator*(); }

    std::unique_ptr<Iovec[]> GetIOVector(
        size_t& iov_count,
        size_t& writable_datapoints,
        size_t max_writable_datapoints,
        const int64_t max_writable_timestamp) const {
      return impl->GetIOVector(
          pos,
          iov_count,
          writable_datapoints,
          max_writable_datapoints,
          max_writable_timestamp);
    }

    size_t Pos() const { return pos; }

   private:
    std::unique_ptr<IteratorImpl> impl;
    size_t pos;
  };

  virtual size_t Size() const = 0;
  virtual Iterator begin() = 0;
  virtual Iterator end() = 0;
};


}  // namespace db
}  // namespace vqro

#endif  // VQRO_DB_DATAPOINT_BUFFER_H
