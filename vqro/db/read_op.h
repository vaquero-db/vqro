#ifndef VQRO_DB_READ_OP_H
#define VQRO_DB_READ_OP_H

#include <cstdint>
#include "vqro/db/datapoint.h"


namespace vqro {
namespace db {


// Holds state for a Read() call so we can Read() chunk by chunk. This allows
// interleaving of a large reads with other operations and the ability to
// resume a read later if we run out of buffer space. This is needed to
// prevent blocking an IO thread if there is a slow client doing a large read.
struct ReadOperation {
  ReadOperation(int64_t start,
                int64_t end,
                int64_t limit,
                bool latest,
                Datapoint* buf,
                size_t siz) :
      start_time(start),
      next_time(start),
      end_time(end),
      datapoint_limit(limit),
      prefer_latest(latest),
      buffer(buf),
      buffer_size(siz),
      cursor(buf) {}

  const int64_t start_time;   // Lower bound of all timestamps to read
  int64_t next_time;          // Lower bound of next timestamp to read
  const int64_t end_time;     // Read is complete when next_time reaches end_time.
  int64_t datapoint_limit;    // Limits total number of datapoints we will read
  bool prefer_latest;         // Specifies if we want first or last N datapoints

  // All underlying read operations populate our buffer, and we track how
  // much we've already read with a cursor.
  Datapoint* const buffer;
  const size_t buffer_size;
  Datapoint* cursor;

  void Advance() {
    next_time = cursor->timestamp + (cursor->duration ? cursor->duration : 1);
    cursor++;
  }

  void Append(Datapoint& point) {
    *cursor = point;
    Advance();
  }

  size_t DatapointsInBuffer() { return cursor - buffer; }
  size_t SpaceLeft() { return buffer_size - DatapointsInBuffer(); }
  void ClearBuffer() { cursor = buffer; }
  bool Complete() { return next_time >= end_time; }
};


} // namespace db
} // namespace vqro

#endif // VQRO_DB_READ_OP_H
