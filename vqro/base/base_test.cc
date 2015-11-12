#include <fcntl.h>
#include "vqro/base/base.h"
#include "gtest/gtest.h"


namespace {

using namespace vqro;


TEST(BaseTest, ReadProcStatWorks) {
  ProcStat stats;
  ASSERT_EQ(stats.open_fd, 0);

  int fd = open("/dev/null", O_RDONLY);
  ASSERT_NE(fd, -1);

  int ret = ReadProcStat(&stats);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(stats.open_fd, 0);
  close(fd);
}


}  // namespace
