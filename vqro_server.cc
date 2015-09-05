#include <iostream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "vqro/rpc/vqro.grpc.pb.h"
#include "vqro/base/fileutil.h"
#include "vqro/db/db.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using vqro::rpc::VaqueroStorage;
using vqro::rpc::StatusMessage;
using vqro::rpc::WriteOperation;
using vqro::rpc::ReadOperation;
using vqro::rpc::ReadResult;
using vqro::rpc::SeriesQuery;
using vqro::rpc::SearchSeriesResult;
using vqro::rpc::LabelsQuery;
using vqro::rpc::SearchLabelsResult;
using vqro::db::Database;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;


DEFINE_string(listen_ip, "127.0.0.1", "IP to listen on");
DEFINE_int32(listen_port, 7950, "Port to listen on");
DEFINE_string(data_directory, "", "Directory that stores datapoints.");
DEFINE_int32(datapoint_file_mode, 0644, "Permissions for datapoint files.");
DEFINE_int32(max_sparse_file_size, 1 << 22, "Max size in bytes of a sparse file.");


class VaqueroStorageServiceImpl final : public VaqueroStorage::Service {
 public:
  VaqueroStorageServiceImpl(Database* _db)
      : db(_db) {}

 private:
  Database* const db;

  Status WriteDatapoints(ServerContext* context,
                         ServerReaderWriter<StatusMessage, WriteOperation>* stream) override {
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
    auto read_series = [&] (SearchSeriesResult& search_result) {
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
      for (auto series : search_result.matching_series()) {
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

    // Kick off the search, read, respond sequence.
    db->SearchSeries(read_op->query(),
                     read_series);

    LOG(INFO) << "ReadDatapoints() matched " << matched_series
              << " series and read " << datapoints_read << " datapoints.";
    return Status::OK;
  }


  Status SearchSeries(ServerContext* context,
                      const SeriesQuery* query,
                      ServerWriter<SearchSeriesResult>* writer) override {

    auto respond = [&] (SearchSeriesResult& result) { writer->Write(result); };
    try {
      db->SearchSeries(*query, respond);
    } catch (vqro::db::DatabaseError& err) {
      LOG(ERROR) << "DatabaseError during SearchSeries: " << err.message; //XXX StatusMessage
    }
    return Status::OK;
  }


  Status SearchLabels(ServerContext* context,
                      const LabelsQuery* query,
                      ServerWriter<SearchLabelsResult>* writer) override {

    auto respond = [&] (SearchLabelsResult& result) { writer->Write(result); };
    try {
      db->SearchLabels(*query, respond);
    } catch (vqro::db::DatabaseError& err) {
      LOG(ERROR) << "DatabaseError during SearchLabels: " << err.message; //XXX StatusMessage
    }
    return Status::OK;
  }
};


void RunServer(string server_address) {
  Database db(FLAGS_data_directory);
  VaqueroStorageServiceImpl service(&db);

  LOG(INFO) << "Working directory: " << vqro::GetCurrentWorkingDirectory();
  LOG(INFO) << "Database initialized in data_directory=" << FLAGS_data_directory;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}


int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (FLAGS_data_directory.empty()) {
    cerr << "You must specify --data_directory" << endl;
    return 1;
  }

  string server_address(FLAGS_listen_ip + ":" + to_string(FLAGS_listen_port));

  grpc_init();
  RunServer(server_address);
  grpc_shutdown();

  return 0;
}
