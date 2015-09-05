#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>

#include <gflags/gflags.h>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/db/sparse_file.h"
#include "vqro/db/datapoint_directory.h"


DECLARE_int32(datapoint_file_mode);  // 0644
DECLARE_int32(max_sparse_file_size); // 2**22 (4MB) seems like a reasonable default, fits ~175k points


namespace vqro {
namespace db {


string SparseFile::GetPath() {
  return dir->path + "/" + to_string(min_timestamp) + "-" + to_string(max_timestamp);
}


std::unique_ptr<DatapointFile> SparseFile::FromFilename(
    DatapointDirectory* dir,
    char* filename) {
  // sparse filename format is "<min_timestamp>-<max_timestamp>"
  std::unique_ptr<DatapointFile> failed_to_parse(nullptr);
  char* endptr;
  int64_t min;
  int64_t max;

  min = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '-')
    return failed_to_parse;

  filename = endptr + 1;
  max = strtoll(filename, &endptr, 10);
  if (endptr == filename || *endptr != '\0')
    return failed_to_parse;

  return std::unique_ptr<DatapointFile>(
    static_cast<DatapointFile*>(new SparseFile(dir, min, max))
  );
}


void SparseFile::Read(ReadOperation& read_op)
{
  // We read the entire file into memory (up to our safety limit).
  LOG(INFO) << "SparseFile::Read() opening file " << GetPath();
  FileHandle file(GetPath(), O_RDONLY);
  if (file.fd == -1)
    throw IOErrorFromErrno("SparseFile::Read open() failed");

  int file_size = GetFileSize(file.path);
  if (file_size > FLAGS_max_sparse_file_size) {
    LOG(ERROR) << "SparseFile::Scan() oversized file " << file.path << " ignoring some datapoints.";
    //TODO: Signal that the file should be split up asap
  }
  int num_points = std::min(file_size, FLAGS_max_sparse_file_size) / DATAPOINT_SIZE;
  std::unique_ptr<vector<Datapoint>> datapoints =
      ReadValues<Datapoint>(file, num_points);

  // Datapoint ordering is based on timestamp only, duration is ignored. Thus
  // doing a stable_sort will preserve the order in which different datapoints
  // with the same timestamp were written in, making it easier to find the right one.
  std::stable_sort(datapoints->begin(), datapoints->end());

  // Search for the first datapoint with timestamp >= read_start
  Datapoint search_point = Datapoint(read_op.next_time, 0.0, 0);
  auto it = std::lower_bound(datapoints->begin(), datapoints->end(), search_point);
  auto original = it;  // Track starting point of lookahead
  auto next = it;      // Next datapoint with a greater timestamp

  // Copy point by point into the buffer until we're at the end. This is trivial
  // except for complexity incurred in filtering out multiple datapoints with
  // the same timestamp. Such is the cost of an efficient append-only write path.
  while (it != datapoints->end() &&
         it->timestamp >= read_op.next_time &&
         it->timestamp < read_op.end_time &&
         read_op.SpaceLeft() &&
         !read_op.Complete())
  {
    // Skip datapoints that have been updated by subsequent datapoints. Durations
    // are immutable so for a given timestamp we want the last-written datapoint
    // that has the originally-written duration for that timestamp.
    original = it;
    next = it + 1;
    while (next != datapoints->end() &&
           next->timestamp == original->timestamp)
      next++;

    // All datapoints with the same timestamp as *original now lie between
    // original and next. We scan backwards from next and stop as soon as we
    // hit a datapoint with the original duration.
    it = next - 1;
    while (it != original &&
           it->duration != original->duration)
      it--;

    read_op.Append(*it);
    it = next;
  }
}


size_t SparseFile::Write(const WriteOperation& write_op) {
  // Find the max_timestamp we'll be writing
  int64_t buffer_max_timestamp = max_timestamp;
  for (auto it = write_op.cursor; it != write_op.buffer->end(); it++)
    if (it->timestamp > buffer_max_timestamp)
      buffer_max_timestamp = std::min(it->timestamp,
                                      write_op.max_writable_timestamp);

  FileHandle file(GetPath(),
                  O_WRONLY|O_CREAT|O_APPEND,
                  FLAGS_datapoint_file_mode);
  if (file.fd == -1)
    throw IOErrorFromErrno("SparseFile::Write open() failed");

  size_t iov_count = 0;
  size_t writable_datapoints = 0;
  auto iov = write_op.GetIOVector(iov_count, writable_datapoints);
  if (writable_datapoints)
    WriteVector(file, iov.get(), iov_count);

  // If we've increased our max_timestamp we have to rename the file.
  if (buffer_max_timestamp > max_timestamp) {
    max_timestamp = buffer_max_timestamp;

    string new_path = GetPath();
    LOG(INFO) << "SparseFile::Write() renaming " << file.path
              << " to " << new_path;
    if (rename(file.path.c_str(), new_path.c_str()) == -1)
      throw IOErrorFromErrno("SparseFile::Write rename() failed");
  }

  return writable_datapoints;
}


} // namespace db
} // namespace vqro
