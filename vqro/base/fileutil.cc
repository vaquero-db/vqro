#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"


namespace vqro {


string GetCurrentWorkingDirectory() {
  thread_local static char working_dir[4096];
  if (getcwd(working_dir, 4096) == NULL)
    throw IOErrorFromErrno("getcwd() failed");
  return string(working_dir);
}


bool FileExists(string path) {
  struct stat my_stats;
  return stat(path.c_str(), &my_stats) != -1;
}


int GetFileSize(string path, bool error_returns_zero/*=false*/) {
  struct stat my_stats;
  if (stat(path.c_str(), &my_stats) == -1) {
    if (error_returns_zero)
      return 0;
    throw IOErrorFromErrno("GetFileSize stat() failed on path=" + path);
  }

  return my_stats.st_size;
}


void CreateDirectory(string dir_path) {
  struct stat my_stats;

  if (stat(dir_path.c_str(), &my_stats) == 0) {
    if (!S_ISDIR(my_stats.st_mode))
      throw IOError(dir_path + " is not a directory");
    return; // Directory already exists
  }

  if (mkdir(dir_path.c_str(), 0755) == -1) {
    if (errno == ENOENT && dir_path.rfind("/") != string::npos) {
      string parent_dir = dir_path.substr(0, dir_path.rfind("/"));
      CreateDirectory(parent_dir);
      CreateDirectory(dir_path);
    } else {
      throw IOErrorFromErrno("CreateDirectory mkdir() failed");
    }   
  } else {
    LOG(INFO) << "Created directory: " << dir_path;
  }
}


void WriteVector(FileHandle file, Iovec* iov, size_t iov_count) {
  int written;
  while (iov_count) {
    written = writev(file.fd, iov, iov_count);
    LOG(INFO) << "writev() wrote " << written << " bytes";

    if (written < 0) {
      if (errno == EINTR) continue;
      throw IOErrorFromErrno("WriteVector writev() failed");
    } else {
      // Advance the iov pointer to reflect what we just wrote
      while (iov_count && iov->iov_len <= static_cast<unsigned int>(written)) {
        written -= iov->iov_len;
        iov++;
        iov_count--;
      }
      // If we partially wrote an Iovec, update its pointer and length.
      if (iov_count && written) {
        iov->iov_base = static_cast<char*>(iov->iov_base) + written;
        iov->iov_len -= written;
      }
    }
  }
}


} // namespace vqro
