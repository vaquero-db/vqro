#include <iostream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "vqro/base/base.h"
#include "vqro/base/fileutil.h"
#include "vqro/db/db.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/search.grpc.pb.h"
#include "vqro/rpc/storage.grpc.pb.h"
#include "vqro/rpc/search.h"
#include "vqro/rpc/storage.h"


DEFINE_string(listen_ip, "127.0.0.1", "IP to listen on");
DEFINE_int32(listen_port, 7950, "Port to listen on");
DEFINE_bool(h, false, "Print help on important flags");
DEFINE_string(data_directory, "", "Directory that stores datapoints.");
DEFINE_int32(datapoint_file_mode, 0644, "Permissions for datapoint files.");
DEFINE_int32(max_sparse_file_size, 1 << 22, "Max size in bytes of a sparse file.");

using std::to_string;


void RunServer(string server_address) {
  vqro::db::Database db(FLAGS_data_directory);
  vqro::rpc::VaqueroStorageServiceImpl storage_service(&db);
  vqro::rpc::VaqueroSearchServiceImpl search_service(db.search_engine.get());

  LOG(INFO) << "Working directory: " << vqro::GetCurrentWorkingDirectory();
  LOG(INFO) << "Database initialized in data_directory=" << FLAGS_data_directory;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&storage_service);
  builder.RegisterService(&search_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}


int main(int argc, char** argv) {
  google::SetUsageMessage("");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  if (FLAGS_h) {
    google::ShowUsageWithFlagsRestrict(
        google::ProgramInvocationShortName(), __FILE__);
    return 0;
  }

  if (FLAGS_data_directory.empty()) {
    std::cerr << "You must specify --data_directory" << std::endl;
    return 1;
  }

  string server_address(FLAGS_listen_ip + ":" + to_string(FLAGS_listen_port));

  grpc_init();
  RunServer(server_address);
  grpc_shutdown();

  return 0;
}
