#ifndef VQRO_BASE_FILEUTIL_H
#define VQRO_BASE_FILEUTIL_H

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include "vqro/base/base.h"


namespace vqro {

using std::string;
using std::vector;
using Iovec = struct iovec;


// Filesystem utility functions
string GetCurrentWorkingDirectory();
bool FileExists(string path);
int GetFileSize(string path, bool error_returns_zero=false);
void CreateDirectory(string dir_path);


// RAII wrapper for file descriptors
class FileHandle {
 public:
  string path = "";
  int fd = -1;

  FileHandle(string _path, int flags) : path(_path) {
    fd = open(path.c_str(), flags);
  }

  FileHandle(string _path, int flags, mode_t mode) : path(_path) {
    fd = open(path.c_str(), flags, mode);
  }

  ~FileHandle() {
    if (fd != -1)
      close(fd);
  }
};


// RAII wrapper for directory streams
class DirectoryHandle {
 public:
  string path = "";
  DIR* stream = NULL;

  DirectoryHandle(string _path) : path(_path) {
    stream = opendir(path.c_str());
  }

  ~DirectoryHandle() {
    if (stream != NULL)
      closedir(stream);
  }
};


// File descriptor I/O
void WriteVector(FileHandle file, Iovec* iov, size_t iov_len);

template <class T>
std::unique_ptr<vector<T>> ReadValues(FileHandle file, int len) {
  std::unique_ptr<vector<T>> buffer(new vector<T>(len));
  char* start = reinterpret_cast<char*>(buffer->data());
  char* end = start + len * sizeof(T);
  int values_read = 0;
  int bytes_read;

  while (start < end) {
    bytes_read = read(file.fd, start, end - start);

    if (bytes_read == -1) {
      if (errno == EINTR) continue;
      throw IOErrorFromErrno("ReadValues read() failed");
    } else if (bytes_read == 0) {
      break;
    } else {
      values_read += bytes_read / sizeof(T);
      start += bytes_read - (bytes_read % sizeof(T));
    }
  }
  if (values_read < len)
    buffer->resize(values_read);
  return buffer;
}


template <class T>
void WriteValues(FileHandle file, T* buffer, size_t len) {
  char* ptr = reinterpret_cast<char*>(buffer);
  size_t to_write = len * sizeof(T);
  int written;

  while (to_write) {
    written = write(file.fd, ptr, to_write);

    if (written == -1) {
      if (errno == EINTR) continue;
      throw IOErrorFromErrno("WriteValues write() failed");
    } else {
      ptr += written;
      to_write -= written;
    }
  }
}

} // namespace vqro

#endif // VQRO_BASE_FILEUTIL_H
