#include "vqro/base/base.h"

#include "syscall.h"
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>

#include <csignal>
#include <ctime>
#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>


namespace vqro {

static bool _signal_handlers_setup = false;
static std::mutex  _signal_handler_mutex;
const long pagesize = sysconf(_SC_PAGESIZE);


void SetupSignalHandlers() {
  std::lock_guard<std::mutex> lck(_signal_handler_mutex);
  if (_signal_handlers_setup) return;
  LOG(INFO) << "Setting up signal handlers";
  // Some bugs, like the existence of SIGPIPE, are so old that it is a "bug" to
  // not explicitly work around them. It's not going anywhere anytime soon and
  // that is truly a shame.
  signal(SIGPIPE, SIG_IGN);
  _signal_handlers_setup = true;
}


string GetEnvVar(string var_name) {
  char* value = getenv(var_name.c_str());
  if (value == NULL)
    return string("");
  return string(value);
}


const string GetThreadId() {
  // https://sourceware.org/bugzilla/show_bug.cgi?id=6399
  // Really enlightening discussion about difference between pthread IDs and
  // TIDs. On Linux they happen to be 1:1, though neither API guarantees this
  // behavior. I'm inclined to be practical not portable and just use TIDs.
  thread_local static const string tid = to_string(syscall(SYS_gettid));
  return tid;
}


void PrintBacktrace() {
  void *array[512];
  size_t size = backtrace(array, 512);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}


// Resource usage
int ReadProcStat(ProcStat* s) {
  // Measure our own CPU ticks and memory usage
  FILE *f = fopen("/proc/self/stat", "r"); //TODO FileHandle handle; FILE* stream = fdopen(handle.fd, "r"); (C++-ify it basically)
  if (f == NULL) {
    perror("fopen(/proc/self/stat)");
    return -1;
  }
  long rss_pages;
  if (fscanf(f,
             "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
             "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
             &s->user_ticks,
             &s->sys_ticks,
             &s->child_user_ticks,
             &s->child_sys_ticks,
             &s->vsize_bytes,
             &rss_pages) == EOF) {
    perror("fscanf(/proc/self/stat");
    fclose(f);
    return -1;
  }
  fclose(f);
  s->rss_bytes = rss_pages * getpagesize();

  // Measure system-wide CPU ticks
  f = fopen("/proc/stat", "r");
  if (f == NULL) {
    perror("fopen(/proc/stat)");
    return -1;
  }
  unsigned long user_ticks;
  unsigned long nice_ticks;
  unsigned long sys_ticks;
  unsigned long idle_ticks;
  if (fscanf(f,
             "cpu %lu %lu %lu %lu",
             &user_ticks,
             &nice_ticks,
             &sys_ticks,
             &idle_ticks) == EOF) {
    perror("fscanf(/proc/stat");
    fclose(f);
    return -1;
  }
  fclose(f);
  s->total_ticks = user_ticks + nice_ticks + sys_ticks + idle_ticks;

  // Count our open FDs
  s->open_fd = 0;
  struct dirent entry;
  struct dirent* entry_ptr = &entry;
  DIR *fd_dir = opendir("/proc/self/fd/");
  if (fd_dir == NULL) {
    perror("opendir(/proc/self/fd/");
    return -1;
  }
  while (readdir_r(fd_dir, entry_ptr, &entry_ptr) == 0 &&
         entry_ptr != NULL) s->open_fd++;
  closedir(fd_dir);
  s->open_fd -= 2; // Don't count . and .. entries

  return 0;
}


void LogResourceUsage() {
  static bool first_time = true;
  static ProcStat current;
  static ProcStat last;

  if (first_time) {
    ReadProcStat(&last);
    first_time = false;
    return;
  }

  // Read current data
  if (ReadProcStat(&current) == -1) {
    LOG(ERROR) << "ReadProcStat failed";
    return;
  }
  // Compute deltas since last read
  unsigned long user_ticks_delta = current.user_ticks - last.user_ticks;
  unsigned long sys_ticks_delta = current.sys_ticks - last.sys_ticks;
  unsigned long total_ticks_delta = current.total_ticks - last.total_ticks;
  // Copy current to last
  last = current;
  // Avoid zero division
  if (!total_ticks_delta) return;
  // Compute utilization
  double user_utilization = (double)user_ticks_delta / (double)total_ticks_delta;
  double sys_utilization = (double)sys_ticks_delta / (double)total_ticks_delta;

  LOG(INFO) <<
      "vsize_bytes=" + to_string(current.vsize_bytes) +
      " rss_bytes=" + to_string(current.rss_bytes) +
      " open_fd=" + to_string(current.open_fd) +
      " user_util=" + to_string(user_utilization) +
      " sys_util=" + to_string(sys_utilization);
}


} // namespace vqro
