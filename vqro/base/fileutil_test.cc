#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "gtest/gtest.h"


namespace {

using namespace vqro;


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


TEST(FileutilTest, GetFileSizeWorks) {
  // create a file, confirm i see right size
  string tmpdir = GetEnvVar("TEST_TMPDIR");
  FileHandle file(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC);
  ASSERT_NE(file.fd, -1);

  const char* buf = "testing123";
  size_t buflen = strlen(buf);
  ASSERT_EQ(write(file.fd, buf, buflen), buflen);
  EXPECT_EQ(GetFileSize(file.path), buflen);

  // second arg is return_zero_on_error
  EXPECT_EQ(GetFileSize("/path/to/nowhere", true), 0);
  // otherwise we should get an IOError
  EXPECT_THROW(GetFileSize("/path/to/nowhere", false), IOError);
  // the default is to throw the exception
  EXPECT_THROW(GetFileSize("/path/to/nowhere"), IOError);
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


TEST(FileutilTest, ReadValuesWorks) {
  struct Foo {
    char a = 'a';
    char b = 'B';
    char c = 'c';
  };
  constexpr ssize_t num_foo = 4;
  size_t foo_bytes = num_foo * sizeof(Foo);
  const char* testdata = "XyzXyzXyzXyz";

  string tmpdir = GetEnvVar("TEST_TMPDIR");
  FileHandle wfile(tmpdir + "/testfile", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  ASSERT_EQ(write(wfile.fd, testdata, foo_bytes), foo_bytes);

  FileHandle rfile(wfile.path, O_RDONLY);
  ASSERT_NE(rfile.fd, -1);
  std::unique_ptr<vector<Foo>> foobuf = ReadValues<Foo>(rfile, num_foo);

  // check that we read what we wrote
  for (int i = 0; i < num_foo; i++) {
    EXPECT_EQ(foobuf->at(i).a, 'X');
    EXPECT_EQ(foobuf->at(i).b, 'y');
    EXPECT_EQ(foobuf->at(i).c, 'z');
  }

  ASSERT_EQ(unlink(wfile.path.c_str()), 0);
}


}  // namespace
