#ifndef VQRO_RPC_STORAGE_H
#define VQRO_RPC_STORAGE_H

#include <glog/logging.h>

#include "vqro/base/base.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/storage.grpc.pb.h"
#include "vqro/db/db.h"

using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using vqro::rpc::VaqueroStorage;
using vqro::rpc::StatusMessage;
using vqro::rpc::WriteOperation;
using vqro::rpc::ReadOperation;
using vqro::rpc::ReadResult;
using vqro::rpc::SearchSeriesResults;
using vqro::db::Database;


namespace vqro {
namespace rpc {


class VaqueroStorageServiceImpl final : public VaqueroStorage::Service {
 public:
  VaqueroStorageServiceImpl(Database* _db)
      : db(_db) {}

 private:
  Database* const db;

  Status WriteDatapoints(ServerContext* context,
                         ServerReaderWriter<StatusMessage,WriteOperation>* stream) override {
    WriteOperation op;
    int written = 0;

    LOG(INFO) << "WriteDatapoints() called";
    while (stream->Read(&op)) {
      LOG(INFO) << "Writing " << to_string(op.datapoints_size()) << " datapoints";
      try {
        db->Write(op);
        written += op.datapoints_size();
      } catch (vqro::db::InvalidSeriesProto& err) {
        LOG(WARNING) << "Write failure: " << err.message;
        StatusMessage sm;
        sm.set_text(err.message);
        sm.set_error(true);
        //stream->Write(sm); //TODO fix this with newer grpc
      }
    }
    LOG(INFO) << "Wrote " << written << " datapoints.";
    return Status::OK;
  }

  Status ReadDatapoints(ServerContext* context,
                        const ReadOperation* read_op,
                        ServerWriter<ReadResult>* writer) override {
    ReadResult read_result;
    vqro::rpc::Datapoint* proto_point;
    int matched_series = 0;
    int datapoints_read = 0;

    LOG(INFO) << "ReadDatapoints() called";

    // First we search for matching series, which are handled by this outer lambda.
    auto read_series = [&] (SearchSeriesResults& search_results) {
      // Then we Read() each series, streaming back results with this inner lambda.
      auto respond = [&] (vqro::db::Datapoint* db_points, size_t num_points) {
        read_result.clear_datapoints();
        for (unsigned int i = 0; i < num_points; i++) {
          proto_point = read_result.add_datapoints();
          proto_point->set_timestamp(db_points[i].timestamp);
          proto_point->set_duration(db_points[i].duration);
          proto_point->set_value(db_points[i].value);
        }
        datapoints_read += num_points;
        // Lastly, write a ReadResult back to the client
        writer->Write(read_result);
      };
      // Read() each series that matched the query
      for (auto series : search_results.matches()) {
        matched_series++;
        read_result.Clear();
        *read_result.mutable_series() = series;

        db->Read(series,
                 read_op->start_time(),
                 read_op->end_time(),
                 read_op->datapoint_limit(),
                 read_op->prefer_latest(),
                 respond);
      }
    }; // read_series

    // Kick off the read, respond sequence.
    switch (read_op->selector_case()) {
      case ReadOperation::kQuery:
        try {
          db->search_engine->SearchSeries(read_op->query(),
                                          read_series);
        } catch (vqro::db::SqliteError& err) {
          //TODO: increment an error counter
          LOG(ERROR) << "SqliteError exception doing SearchSeries: " << err.message;
        }
        break;

      case ReadOperation::kList:
        {
          SearchSeriesResults results;
          *results.mutable_matches() = read_op->list().series();
          read_series(results);
        }
        break;

      case ReadOperation::SELECTOR_NOT_SET:
        LOG(ERROR) << "Invalid ReadOperation, selector not set.";
        return Status(StatusCode::INVALID_ARGUMENT, "Selector not specified");
    }

    LOG(INFO) << "ReadDatapoints() matched " << matched_series
              << " series and read " << datapoints_read << " datapoints.";
    return Status::OK;
  }
};


}  // namespace rpc
}  // namespace vqro

#endif  // VQRO_RPC_STORAGE_H
