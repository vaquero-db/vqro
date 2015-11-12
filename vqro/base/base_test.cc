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
  string tmpdir = GetEnvVar("TEST_TMPDIR");
  string path = tmpdir + "/testdir/subdir";
  EXPECT_NO_THROW(CreateDirectory(path));
  ASSERT_TRUE(FileExists(path));
  // Shouldn't fail if the directory already exists
  EXPECT_NO_THROW(CreateDirectory(path));
}


TEST(FileutilTest, CreateDirectoryThrowsOnFailure) {
  string tmpdir = GetEnvVar("TEST_TMPDIR");
  FileHandle file(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC);
  EXPECT_THROW(CreateDirectory(file.path), IOError);
  EXPECT_THROW(CreateDirectory("/dev/null/subnull"), IOError);
  ASSERT_EQ(unlink(file.path.c_str()), 0);
}


TEST(FileutilTest, WriteVectorWorks) {
  // prepare a non-trivial io vector to write
  char data[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  const size_t iov_size = 3;
  Iovec* iov = new Iovec[iov_size];

  iov->iov_base = data + 5;
  iov->iov_len = 3;
  (iov+1)->iov_base = data + 12;
  (iov+1)->iov_len = 5;
  (iov+2)->iov_base = data + 20;
  (iov+2)->iov_len = 2;

  // write the vector
  string tmpdir = GetEnvVar("TEST_TMPDIR");
  FileHandle wfile(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  ASSERT_NE(wfile.fd, -1);
  EXPECT_NO_THROW(WriteVector(wfile, iov, iov_size));

  // re-read file and check contents to be correct.
  FileHandle rfile(wfile.path, O_RDONLY);
  ASSERT_NE(rfile.fd, -1);
  char buf[64];
  const char* expected_contents = "567cdefgkl";
  ASSERT_EQ(read(rfile.fd, buf, 64), 10);
  EXPECT_TRUE(memcmp(buf, expected_contents, 10) == 0);

  delete iov;
  ASSERT_EQ(unlink(wfile.path.c_str()), 0);
}


TEST(FileutilTest, WriteValuesWorks) {
  string tmpdir = GetEnvVar("TEST_TMPDIR");
  FileHandle wfile(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  ASSERT_NE(wfile.fd, -1);

  struct Foo {
    char a = 'a';
    char b = 'B';
    char c = 'c';
  };
  constexpr size_t num_foo = 4;
  Foo foobuf[num_foo];
  EXPECT_NO_THROW(WriteValues<Foo>(wfile, foobuf, num_foo));

  // re-read file and check contents to be correct.
  FileHandle rfile(wfile.path, O_RDONLY);
  ASSERT_NE(rfile.fd, -1);
  char buf[64];
  size_t foo_bytes = num_foo * sizeof(Foo);
  EXPECT_EQ(read(rfile.fd, buf, 64), foo_bytes);
  const char* expected_contents = "aBcaBcaBcaBc";
  EXPECT_TRUE(memcmp(buf, expected_contents, foo_bytes) == 0);

  ASSERT_EQ(unlink(wfile.path.c_str()), 0);
}


}  // namespace
