#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/db/datapoint_directory.h"
#include "vqro/db/datapoint_file.h"
#include "vqro/db/dense_file.h"
#include "vqro/db/sparse_file.h"
#include "vqro/db/read_op.h"
#include "vqro/db/write_op.h"


namespace vqro {
namespace db {


//TODO: support datapoint_limit and prefer_latest
void DatapointDirectory::Read(ReadOperation& read_op) {
  // It is possible that our actual directory does not yet exist because
  // Write() hasn't been called yet.
  if (!filenames_read) {
    try {
      ReadFilenames();
    } catch (IOError& e) {
      PLOG(WARNING) << "DatapointDirectory::ReadFilenames() failed during Read";
      return;
    }
  }

  // file_it will point to the latest datapoint_files member such that its
  // min_timestamp < read_op.next_time
  auto file_it = FindFirstPotentialFile(read_op.next_time);

  while (read_op.SpaceLeft() && !read_op.Complete())
  {
    // Advance file_it to the next file that might have data in our range
    while (file_it != datapoint_files.end() &&
           (*file_it)->max_timestamp < read_op.next_time)
      file_it++;

    // When we run out of files in the read_op's range we're done.
    if (file_it == datapoint_files.end() ||
        (*file_it)->min_timestamp >= read_op.end_time)
      return;

    // DatapointFile::Read() advances read_op.next_time for us
    (*file_it++)->Read(read_op);
  }
}


void DatapointDirectory::Write(WriteOperation& write_op) {
  CreateDirectory(path);

  if (!filenames_read)
    ReadFilenames();

  auto file_it = FindFirstPotentialFile(write_op.cursor->timestamp);

  while (!write_op.Complete()) {

    // See if we can write to an existing file
    if (file_it != datapoint_files.end()) {
      // The max timestamp we can write to a given file is bounded by the
      // min_timestamp of the next file.
      auto next_file = file_it + 1;
      write_op.max_writable_timestamp = (next_file == datapoint_files.end()) ?
          INT64_MAX : (*next_file)->min_timestamp - 1;

      // Skip old files that cannot hold the current datapoint
      if (write_op.max_writable_timestamp < write_op.cursor->timestamp) {
        file_it++;
        continue;
      }

      // If the file doesn't start in the future, attempt a write.
      if (write_op.cursor->timestamp >= (*file_it)->min_timestamp) {
        size_t datapoints_written = (*file_it)->Write(write_op);
        write_op.cursor += datapoints_written; //TODO have WriteOperation::Advance() update its own cursor or something, called by DatapointFile::Write
        file_it++;

        // If we wrote some datapoints we can move on to the next file, otherwise
        // we gotta create a new file.
        if (datapoints_written)
          continue;
      }
    }

    // No existing file can fit the current datapoint, though future files
    // may still exist that can hold later datapoints from our buffer. We know
    // from conditions above that file_it is either at the end or the next
    // future file we may eventually be able to write to.
    write_op.max_writable_timestamp = (file_it == datapoint_files.end()) ?
        INT64_MAX : (*file_it)->min_timestamp - 1;

    // Create a new sparse file.
    std::unique_ptr<DatapointFile> new_file(
      static_cast<DatapointFile*>(
        new SparseFile(this,
                       write_op.cursor->timestamp,
                       write_op.cursor->timestamp)
      )
    );
    write_op.cursor += new_file->Write(write_op);
    file_it = datapoint_files.insert(file_it, std::move(new_file));
    file_it++;
  }
}


vector<std::unique_ptr<DatapointFile>>::iterator
DatapointDirectory::FindFirstPotentialFile(int64_t timestamp)
{
  // Return the last datapoint_files member with min_timestamp <= timestamp.
  int left = 0;
  int middle = 0;
  int right = datapoint_files.size();
  while (left < right) {
    middle = (right - left) / 2;
    if (datapoint_files[middle]->min_timestamp > timestamp)
      right = middle;
    else if (datapoint_files[middle]->min_timestamp < timestamp)
      left = middle + 1;
    else
      break;
  }
  return datapoint_files.begin() + middle;
}


void DatapointDirectory::ReadFilenames() {
  DirectoryHandle dir(path);

  if (dir.stream == NULL)
    throw IOErrorFromErrno("ReadFilenames opendir() failed path=" + path);

  struct dirent64* entry;
  std::unique_ptr<DatapointFile> data_file;
  vector<std::unique_ptr<DatapointFile>> new_files;

  // Now we update our maps with new directory entries
  while ((entry = readdir64(dir.stream)) != NULL) {
    switch (entry->d_type) {
      case DT_REG:
      case DT_LNK:
      case DT_UNKNOWN:

        data_file = DenseFile::FromFilename(this, entry->d_name);
        if (data_file.get() != nullptr) {
          new_files.push_back(std::move(data_file));
          continue;
        }

        data_file = SparseFile::FromFilename(this, entry->d_name);
        if (data_file.get() != nullptr)
          new_files.push_back(std::move(data_file));

        break;

      default: // Ignore uninteresting dirent types
        continue;
    }
  }

  std::sort(new_files.begin(), new_files.end());
  datapoint_files.swap(new_files);
  filenames_read = true;
}


} // namespace db
} // namespace vqro
