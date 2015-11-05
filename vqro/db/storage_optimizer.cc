#include <cfloat>
#include <cmath>
#include "vqro/base/base.h"
#include "vqro/base/floatutil.h"
#include "vqro/db/db.h"
#include "vqro/db/constant_file.h"
#include "vqro/db/dense_file.h"
#include "vqro/db/raw_buffer.h"
#include "vqro/db/storage_optimizer.h"
#include "vqro/db/write_op.h"


// Values in a dense file are 8 bytes. 512 values == 4 kilobytes,
// which is often the filesystem block size. If we're filling at least
// half a block with padding, might as well just make a new file instead.
DEFINE_int32(max_dense_nan_padding,
             256,
             "Maximum number of NANs that may be used to fill in "
             "gaps in a dense datapoint file.");

DEFINE_int32(min_datapoints_for_dense,
             32,
             "Minimum number of datapoints required to assert that the "
             "datapoints are 'dense'.");

DEFINE_int32(min_datapoints_for_constant,
             32,
             "Minimum number of datapoints required to assert that the "
             "datapoints are 'constant'.");


namespace vqro {
namespace db {


bool StorageOptimizer::IsDense(Datapoint* buf, size_t len) {
  if (len < static_cast<uint32_t>(FLAGS_min_datapoints_for_dense))
    return false;

  const int64_t dur = buf->duration ? buf->duration : 1;
  int64_t last_timestamp = buf->timestamp;
  for (unsigned int i = 0; i < len; i++) {
    int64_t delta = buf[i].timestamp - last_timestamp;
    if (buf[i].duration != dur ||
        delta / dur > FLAGS_max_dense_nan_padding ||
        delta % dur)
      return false;

    last_timestamp = buf[i].timestamp;
  }
  return true;
}


bool StorageOptimizer::IsConstant(Datapoint* buf, size_t len) {
  if (len < static_cast<uint32_t>(FLAGS_min_datapoints_for_constant))
    return false;

  const double value = buf->value;
  Datapoint* const end = buf + len;
  while (buf < end && AlmostEquals(buf->value, value))
    buf++;

  return buf == end;
}


void StorageOptimizer::SparseFileTooBig(SparseFile* sparse_file) {
  Series* series = sparse_file->dir->series;
  series->db->GetWorker(series)->Do([=] {
    HandleSparseFileTooBig(*sparse_file);
  });  // Don't wait on the worker, that would result in deadlock.
}


void StorageOptimizer::HandleSparseFileTooBig(SparseFile& sparse_file) {
  LOG(INFO) << "HandleSparseFileTooBig file=" << sparse_file.GetPath();

  //TODO Arena allocation for read buffers
  std::unique_ptr<Datapoint> read_buffer(new Datapoint[FLAGS_sparse_file_max_size]);
  ReadOperation read_op(INT64_MIN,  // start_time
                        INT64_MAX,  // end_time
                        INT64_MAX,  // datapoint_limit
                        true,       // prefer_latest
                        read_buffer.get(),
                        FLAGS_read_buffer_size);

  // If the file is larger than FLAGS_sparse_file_max_size we ignore trailing data.
  sparse_file.Read(read_op);

  if (IsDense(read_op.buffer, read_op.DatapointsInBuffer())) {
    if (IsConstant(read_op.buffer, read_op.DatapointsInBuffer())) {
      ConvertSparseToConstant(sparse_file,
                              read_op.buffer,
                              read_op.DatapointsInBuffer());
    } else {
      ConvertSparseToDense(sparse_file,
                           read_op.buffer,
                           read_op.DatapointsInBuffer());
    }
  }
  // If the file can't be converted to a better format we just leave it be and
  // let the write path add more sparse files.
}


void StorageOptimizer::ConvertSparseToConstant(const SparseFile& sparse_file,
                                               Datapoint* buf,
                                               size_t len)
{
  LOG(INFO) << "Converting SparseFile: " << sparse_file.GetPath();
  ConstantFile new_file(sparse_file.dir,
                        sparse_file.min_timestamp,
                        buf->duration,
                        0,  // count starts at zero, gets increased by Write
                        buf->value);

  RawBuffer rawbuf(buf, len);
  WriteOperation write_op(&rawbuf);
  new_file.Write(write_op);
  if (unlink(sparse_file.GetPath().c_str()) == -1) {
    LOG(ERROR) << "Failed to delete converted sparse file: " << sparse_file.GetPath();
  }
  LOG(INFO) << "Created ConstantFile: " << new_file.GetPath();
  sparse_file.dir->ReadFilenames();
}


void StorageOptimizer::ConvertSparseToDense(const SparseFile& sparse_file,
                                            Datapoint* buf,
                                            size_t len)
{
  LOG(INFO) << "Converting " << sparse_file.GetPath() << " to dense format";
  DenseFile new_file(sparse_file.dir,
                     sparse_file.min_timestamp,
                     buf->duration);

  RawBuffer rawbuf(buf, len);
  WriteOperation write_op(&rawbuf);
  new_file.Write(write_op);
  if (unlink(sparse_file.GetPath().c_str()) == -1) {
    LOG(ERROR) << "Failed to delete converted sparse file: " << sparse_file.GetPath();
  }
  LOG(INFO) << "Created DenseFile: " << new_file.GetPath();
  sparse_file.dir->ReadFilenames();
}


// Other things the optimizer might do:
// - handle ephemeral Series promotion to persisted Series (for event store use case)
// - roll up aggregation
// - expiration, deleting old files
// - truncating/removing corrupt datafiles


}  // namespace db
}  // namespace vqro
