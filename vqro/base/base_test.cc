#include "vqro/base/base.h"
#include "gtest/gtest.h"


TEST(BaseTest, ReadProcStatWorks) {
  vqro::ProcStat stats;
  int ret = vqro::ReadProcStat(&stats);
  EXPECT_EQ(0, ret);
}
