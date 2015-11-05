#ifndef VQRO_BASE_BASE_H
#define VQRO_BASE_BASE_H

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <exception>
#include <ctime>
#include <string>
#include <limits>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <functional>

#include <gflags/gflags.h>
#include <glog/logging.h>


namespace vqro {

using std::string;
using std::to_string;
using std::vector;

using VoidFunc = std::function<void()>;

constexpr double double_nan = std::numeric_limits<double>::quiet_NaN();

extern const long pagesize;

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

#define __FILE_BASENAME__ (strrchr(__FILE__, '/') ? \
                           strrchr(__FILE__, '/') + 1 : __FILE__)

// vqro exceptions
class Error : public std::exception {
 public:
  Error() : message("Generic vqro error") {}
  Error(string msg) : message(msg) {}
  Error(char* msg) : message(msg) {}
  Error(const char* msg) : message(msg) {}
  virtual ~Error() {}

  const char* what() const noexcept { return message.c_str(); }
  string message;
};


class IOError : public Error {
 public:
  IOError() { Error("Generic IOError"); }
  IOError(string msg) : Error(msg) {}
  IOError(char* msg) : Error(msg) {}
  virtual ~IOError() {}
};


inline IOError IOErrorFromErrno(string msg, bool log_it=true) {
  if (log_it)
    PLOG(ERROR) << "IOError: " << msg;

  return IOError(msg + 
      string(" (errno=") +
      to_string(errno) +
      string(" ") +
      strerror(errno) +
      string(")")
  );
}


inline void LogAndThrow(const Error& e) {
  LOG(ERROR) << e.what();
  throw e;
}

#define LOG_AND_RETURN_FALSE(msg) { LOG(ERROR) << msg; return false; }
#define LOGW_AND_RETURN_FALSE(msg) { LOG(WARNING) << msg; return false; }
#define PLOG_AND_RETURN_FALSE(msg) { PLOG(ERROR) << msg; return false; }


inline int64_t TimeInMillis() {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (now.tv_sec * 1000) + (now.tv_nsec / 1000000);
}


inline int64_t TimeInMicros() {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (now.tv_sec * 1000000) + (now.tv_nsec / 1000);
}


void SetupSignalHandlers();
string GetEnvVar(string var_name);
const string GetThreadId();
void PrintBacktrace();


template <class T>
inline string HexString(T i) {
  std::stringstream s;
  s << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
  return s.str();
}


inline void DoNothing() {}


// A ProcStat object stores stats we care about reading from /proc
struct ProcStat {
  unsigned long user_ticks = 0;
  unsigned long sys_ticks = 0;
           long child_user_ticks = 0;
           long child_sys_ticks = 0;
  unsigned long vsize_bytes = 0;
           long rss_bytes = 0;
  unsigned long total_ticks = 0;
  unsigned long open_fd = 0;
};

int ReadProcStat(ProcStat* p);
void LogResourceUsage();


} // namespace vqro

#endif // VQRO_BASE_BASE_H
