#ifndef VQRO_DB_DATAPOINT_H
#define VQRO_DB_DATAPOINT_H

#include <algorithm>
#include <cstdint>
#include <functional>

namespace vqro {
namespace db {


class Datapoint {
 public:
  Datapoint() = default;
  explicit Datapoint(int64_t t, double v, int64_t d) :
    timestamp(t),
    value(v),
    duration(d) {}

  bool operator<(const Datapoint& other) const {
    return timestamp < other.timestamp;
  }

  bool operator==(const Datapoint& rhs) const {
    return (timestamp == rhs.timestamp &&
            value == rhs.value &&
            duration == rhs.duration);
  }

  bool operator!=(const Datapoint& rhs) const {
    return !(*this == rhs);
  }

  int64_t timestamp;
  double value;
  int64_t duration;
};


inline void swap(Datapoint& a, Datapoint& b) {
  std::swap(a.timestamp, b.timestamp);
  std::swap(a.value, b.value);
  std::swap(a.duration, b.duration);
}


using DatapointsFunc = std::function<void(Datapoint*, size_t)>;

constexpr int8_t datapoint_size = sizeof(::vqro::db::Datapoint);

// If this fails, look at your compiler flags. Any three 64-bit types should be
// able to fit into a 24 byte struct without padding. While no code actually
// *depends* on this being true, having any padding in this type is extremely
// wasteful and unnecessary.
static_assert(datapoint_size == 24, "Datapoints are not 24 bytes!!!");


} // namespace db
} // namespace vqro


#endif // VQRO_DB_DATAPOINT_H
