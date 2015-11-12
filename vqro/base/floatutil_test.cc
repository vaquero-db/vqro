#include "vqro/base/base.h"
#include "vqro/base/floatutil.h"
#include "gtest/gtest.h"


namespace {

using namespace vqro;


TEST(FloatutilTest, AlmostEqualsAlmostWorks) {
  // AlmostEquals should return true for very close values
  {
    double d1 = 0.2;
    double d2 = 1 / std::sqrt(5) / std::sqrt(5);
    EXPECT_FALSE(d1 == d2);
    EXPECT_TRUE(AlmostEquals(d1, d2));
  }
  // But not for questionably distant values
  {
    double d1 = 0.1;
    double d2 = 0.100000000000001;  // one more zero makes AlmostEquals true
    EXPECT_FALSE(d1 == d2);
    EXPECT_FALSE(AlmostEquals(d1, d2));
  }
}


}  // namespace
