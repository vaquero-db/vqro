#include <iostream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "vqro/base/fileutil.h"
#include "vqro/control/controller.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/controller.h"
#include "vqro/rpc/search.h"

using grpc::Server;
using grpc::ServerBuilder;

using vqro::rpc::VaqueroControllerServiceImpl;
using vqro::rpc::VaqueroSearchServiceImpl;
using vqro::control::Controller;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;


DEFINE_string(listen_ip, "127.0.0.1", "IP to listen on");
DEFINE_int32(listen_port, 7960, "Port to listen on");
DEFINE_bool(h, false, "Print help on important flags");
DEFINE_string(state_directory, "", "Where state is saved. Important stuff.");


void RunServer(string server_address) {
  Controller controller(FLAGS_state_directory);
  VaqueroControllerServiceImpl controller_service(&controller);
  VaqueroSearchServiceImpl search_service(controller.search_engine.get());

  LOG(INFO) << "Working directory: " << vqro::GetCurrentWorkingDirectory();

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&controller_service);
  builder.RegisterService(&search_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
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

  if (FLAGS_state_directory.empty()) {
    cerr << "You must specify --state_directory" << endl;
    return 1;
  }
  string server_address(FLAGS_listen_ip + ":" + to_string(FLAGS_listen_port));

  grpc_init();
  RunServer(server_address);
  grpc_shutdown();

  return 0;
}
