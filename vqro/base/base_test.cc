#include <thread>
#include <chrono>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/base/worker.h"
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


TEST(WorkerTest, WorkerDoesWorkAndStops) {
  WorkerThread worker {};
  bool work_got_done = false;

  worker.Start();
  worker.Do([&work_got_done] {
    work_got_done = true;
  }).wait();
  worker.Stop().wait();

  EXPECT_TRUE(work_got_done);
  EXPECT_FALSE(worker.Alive());
}


TEST(WorkerTest, WorkerRefusesTooMuchWork) {
  WorkerThread worker {};
  std::promise<void> will_start;
  std::future<void> work_started = will_start.get_future();
  bool keep_working = true;

  auto some_work = [&] {
    will_start.set_value();
    while (keep_working)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  };

  FLAGS_worker_task_queue_limit = 2;
  worker.Start();

  ASSERT_EQ(worker.TasksQueued(), 0);
  worker.Do(some_work);
  work_started.wait();  // so we know our blocking task is not queued
  ASSERT_EQ(worker.TasksQueued(), 0);

  worker.Do(some_work);
  ASSERT_EQ(worker.TasksQueued(), 1);
  auto last_queued = worker.Do(some_work);
  ASSERT_EQ(worker.TasksQueued(), 2);

  EXPECT_THROW(worker.Do(some_work), WorkerThreadTooBusy);

  keep_working = false;
  last_queued.wait();
  EXPECT_EQ(worker.TasksQueued(), 0);
  worker.Stop().wait();

  EXPECT_FALSE(worker.Alive());
}


TEST(FileutilTest, FileHandleDestructorClosesFD) {
  struct stat s;
  int fd;
  {
    FileHandle file("/dev/null", O_RDONLY);
    fd = file.fd;
    ASSERT_NE(fd, -1);
    EXPECT_EQ(lseek(fd, 0, SEEK_SET), 0);
  }
  // fd should be closed
  EXPECT_EQ(fstat(fd, &s), -1);
  EXPECT_EQ(errno, EBADF);
}


TEST(FileutilTest, DirectoryHandleDestructorClosesStream) {
  DIR* stream;
  struct stat s;
  int fd;
  {
    DirectoryHandle dir("/");
    stream = dir.stream;
    ASSERT_TRUE(stream != NULL);
    // We can't tell if the DIR* is closed or not, but we can tell if the fd
    // associated with the DIR* is.
    ASSERT_NE(fd = dirfd(stream), -1);
    EXPECT_TRUE(readdir(stream) != NULL);
  }
  // fd should be closed
  EXPECT_EQ(fstat(fd, &s), -1);
  EXPECT_EQ(errno, EBADF);
}


TEST(FileutilTest, CreateDirectoryWorks) {
  string tmpdir(getenv("TEST_TMPDIR"));
  string path =  tmpdir + "/testdir/subdir";
  EXPECT_NO_THROW(CreateDirectory(path));
  ASSERT_TRUE(FileExists(path));
  // Shouldn't fail if the directory already exists
  EXPECT_NO_THROW(CreateDirectory(path));
}


TEST(FileutilTest, CreateDirectoryThrowsOnFailure) {
  string tmpdir(getenv("TEST_TMPDIR"));
  FileHandle file(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC);
  EXPECT_THROW(CreateDirectory(file.path), IOError);
  EXPECT_THROW(CreateDirectory("/dev/null/subnull"), IOError);
}


}  // namespace
