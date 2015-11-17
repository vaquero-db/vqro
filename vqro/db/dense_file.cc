#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <memory>

#include <gflags/gflags.h>

#include "vqro/base/fileutil.h"
#include "vqro/db/dense_file.h"
#include "vqro/db/datapoint_directory.h"


DEFINE_int32(max_dense_nan_gap,
             256,
             "How many NAN datapoints will be padded before creating a new file.");

namespace vqro {
namespace db {


string DenseFile::GetPath() const {
  return dir->path + "/" + to_string(min_timestamp) + "@" + to_string(duration);
}


std::unique_ptr<DatapointFile> DenseFile::FromFilename(
    DatapointDirectory* dir,
    char* filename)
{
  // dense filename format is "<start_time>@<duration>"
  std::unique_ptr<DatapointFile> failed_to_parse(nullptr);
  char* endptr;
  int64_t start_time;
  int64_t duration;

  start_time = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '@')
    return failed_to_parse;

  filename = endptr + 1;
  duration = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '\0')
    return failed_to_parse;

  return std::unique_ptr<DatapointFile>(
    static_cast<DatapointFile*>(new DenseFile(dir, start_time, duration))
  );
}


void DenseFile::Read(ReadOperation& read_op) const {
  if (read_op.next_time % duration)
    read_op.next_time = (read_op.next_time / duration + 1) * duration;

  if (max_timestamp < read_op.next_time)
    return;

  FileHandle file(GetPath(), O_RDONLY);
  if (file.fd == -1)
    throw IOErrorFromErrno("DenseFile::Read open() failed");

  off64_t offset = (read_op.next_time - min_timestamp) / duration * dense_datapoint_size;
  if (lseek(file.fd, offset, SEEK_SET) == -1)
    throw IOErrorFromErrno("DenseFile::Read lseek() failed");

  int64_t read_end_time = std::min(max_timestamp, read_op.end_time);
  int64_t datapoints_to_read = (read_end_time - read_op.next_time) / duration;
  std::unique_ptr<vector<double>> values = ReadValues<double>(
      file, datapoints_to_read);

  for (double value : *values) {
    if (std::isnan(value)) continue;
    read_op.cursor->timestamp = read_op.next_time;
    read_op.cursor->value = value;
    read_op.cursor->duration = duration;
    read_op.Advance();
  }
}


size_t DenseFile::Write(const WriteOperation& write_op) {
  size_t datapoints_to_write = write_op.WritableDatapoints();
  if (!datapoints_to_write)
    return 0;

  // Figure out how much NAN padding is needed, if any.
  auto it = write_op.cursor;
  unsigned int num_nans = 0;

  if (it->timestamp > max_timestamp)  // avoid underflow
    num_nans = (it->timestamp - max_timestamp) / duration;

  if (num_nans > static_cast<unsigned int>(FLAGS_max_dense_nan_gap))
    return 0;

  datapoints_to_write += num_nans;
  std::unique_ptr<double[]> values(new double[datapoints_to_write]);

  unsigned int i = 0;
  for (; i < num_nans; i++)
    values[i] = double_nan;

  for (; i < datapoints_to_write; i++)
    values[i] = (it++)->value;
  it = write_op.cursor;

  FileHandle file(GetPath(),
                  O_WRONLY|O_CREAT,
                  FLAGS_datapoint_file_mode);
  if (file.fd == -1)
    throw IOErrorFromErrno("DenseFile::Write open() failed");

  off_t offset = std::min(it->timestamp, max_timestamp) / duration * dense_datapoint_size;
  if (offset) {
    if (lseek(file.fd, offset, SEEK_SET) == -1)
      throw IOErrorFromErrno("DenseFile::Write lseek() failed offset=" +
                             to_string(offset));
  }
  WriteValues<double>(file, values.get(), datapoints_to_write);
  max_timestamp += datapoints_to_write * duration;
  return datapoints_to_write - num_nans;
}


} // namespace db
} // namespace vqro
