#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <memory>

#include "vqro/base/fileutil.h"
#include "vqro/db/constant_file.h"
#include "vqro/db/datapoint_directory.h"


namespace vqro {
namespace db {


string ConstantFile::GetPath() const {
  return dir->path + "/" + to_string(min_timestamp) +
         "@" + to_string(duration) +
         "x" + to_string(count);
}


std::unique_ptr<DatapointFile> ConstantFile::FromFilename(
    DatapointDirectory* dir,
    char* filename)
{
  // constant filename format is "<start_time>@<duration>x<count>=<value>"
  std::unique_ptr<DatapointFile> failed_to_parse(nullptr);
  char* endptr;
  int64_t start_time;
  int64_t duration;
  int64_t count;
  double value;

  start_time = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '@')
    return failed_to_parse;

  filename = endptr + 1;
  duration = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != 'x')
    return failed_to_parse;

  filename = endptr + 1;
  count = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '=')
    return failed_to_parse;

  filename = endptr + 1;
  value = strtod(filename, &endptr);
  if (endptr == filename || *endptr != '\0')
    return failed_to_parse;

  if (std::isnan(value)) {
    LOG(ERROR) << "ConstantFile with value=NAN in " << dir->path;
    return failed_to_parse;
  }

  return std::unique_ptr<DatapointFile>(
    static_cast<DatapointFile*>(
      new ConstantFile(dir, start_time, duration, count, value))
  );
}


void ConstantFile::Read(ReadOperation& read_op) const {
  if (read_op.next_time % duration)
    read_op.next_time = (read_op.next_time / duration + 1) * duration;

  if (max_timestamp < read_op.next_time)
    return;

  int64_t read_end_time = std::min(max_timestamp, read_op.end_time);
  int64_t datapoints_to_read = (read_end_time - read_op.start_time) / duration;

  while (datapoints_to_read--) {
    read_op.cursor->timestamp = read_op.next_time;
    read_op.cursor->value = value;
    read_op.cursor->duration = duration;
    read_op.Advance();
  }
}


size_t ConstantFile::Write(const WriteOperation& write_op) {
  size_t datapoints_to_write = write_op.WritableDatapoints();
  if (!datapoints_to_write)
    return 0;

  string old_path = GetPath();
  count += datapoints_to_write;
  max_timestamp = min_timestamp + count * duration;
  if (rename(old_path.c_str(), GetPath().c_str()) == -1)
    throw IOErrorFromErrno("ConstantFile::Write rename() failed");

  return datapoints_to_write;
}


} // namespace db
} // namespace vqro
