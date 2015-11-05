#include <functional>
#include <memory>

#include "vqro/base/base.h"
#include "vqro/db/series.h"
#include "vqro/db/db.h"
#include "vqro/db/write_op.h"

namespace vqro {
namespace db {


void Series::Init() {
  string series_dir = HexString<size_t>(keyint);
  series_dir.insert(series_dir.begin() + 4, '/');

  data_dir.reset(new DatapointDirectory(
      this, db->GetDataDirectory() + "datapoints/" + series_dir + "/"));
}


void Series::Write(vqro::rpc::WriteOperation& op) 
{
  write_buffer->Append(op);
}


void Series::Read(ReadOperation& read_op) {
  // First we read any datapoints stored on disk.
  data_dir->Read(read_op);

  // Next we append datapoints from our write_buffer onto the read_op buffer.
  // We only guarantee that the write buffer datapoints are merged into read
  // results for *new* datapoints. We do *not* guarantee that write buffer
  // datapoints covering the past will be reflected in read results until
  // those points are flushed to disk. While it is certainly feasible to support
  // fancier merging, it is far more complex and of little utility.
  if (!write_buffer->IsSorted())
    write_buffer->Sort();

  for (auto point : *write_buffer) {
    if (read_op.Complete() || !read_op.SpaceLeft())
      break;

    if (point.timestamp >= read_op.next_time &&
        point.timestamp < read_op.end_time)
      read_op.Append(point);
  }

  // If we didn't fill the read_op buffer then there is no more work we can
  // do, so we force completion.
  if (read_op.SpaceLeft())
    read_op.next_time = read_op.end_time;
}


size_t Series::DatapointsBuffered() {
  return write_buffer->Size();
}


void Series::FlushBufferedDatapoints() {
  if (write_buffer->IsEmpty())
    return;

  if (!write_buffer->IsSorted())
    write_buffer->Sort();

  WriteOperation write_op(write_buffer.get());
  data_dir->Write(write_op);
  write_buffer->Clear();
}


} // namespace db
} // namespace vqro
