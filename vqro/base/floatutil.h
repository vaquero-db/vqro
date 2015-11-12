#ifndef VQRO_BASE_FLOATUTIL_H
#define VQRO_BASE_FLOATUTIL_H
// Bruce Dawson is awesome. Required reading before modifying this code:
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// https://en.wikipedia.org/wiki/Unit_in_the_last_place
//
// As tempting as it is to copy-pasta a solution to this sort of thing, I feel
// the need to understand what makes it complicated and to risk trying to get
// it right.

#include <sys/types.h>
#include <cmath>
#include <cstdint>
#include <limits>


namespace vqro {


static constexpr int MAX_ULPS_DIFF = 4;


bool AlmostEquals(const double a, const double b) {
  // NANs never compare equal to anything, even themselves.
  if (std::isnan(a) || std::isnan(b))
    return false;

  // Near zero there is no use in trying to count ULPs because just about any
  // value you arrive at through arithmetical operations will be very far from
  // zero in terms of ULPs. That is to say, rounding errors are the devil.
  // So to compensate we use an absolute epsilon. For now I'm going with
  // 3 * FLT_EPSILSON, which is 3 * 1.19e-7 for me. We'll see. Note that this
  // handles the 0.0 == -0.0 case.
  if (fabs(a - b) < std::numeric_limits<double>::epsilon())
    return true;

  // Casting to an unsigned integral type makes it really easy to do
  // bitwise operations.
  const uint64_t a_bits = *reinterpret_cast<const uint64_t*>(&a);
  const uint64_t b_bits = *reinterpret_cast<const uint64_t*>(&b);

  // If signs are different only zeroes can be equal, which would've already
  // returned from the epsilon comparison above.
  static const uint64_t sign_mask = 1UL << 63;
  if ((a_bits & sign_mask) != (b_bits & sign_mask))
    return false;

  // Now we want to measure how many possible float values lie between a and b.
  // It turns out that adjacent float values of the same sign differ in binary
  // representation from one another the same way adjacent unsigned integers do.
  // So if we consider the bits of our floats as unsigned ints we can simply
  // measure the distance between the two ints and know that this also tells us
  // how many possible float values lie between our floats.
  return ((a_bits > b_bits) ? a_bits - b_bits : b_bits - a_bits) <= MAX_ULPS_DIFF;
}


}  // namespace vqro

#endif  // VQRO_BASE_FLOATUTIL_H
