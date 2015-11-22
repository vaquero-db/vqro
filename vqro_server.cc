#include <csignal>
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
using std::lock_guard;
using std::mutex;
using vqro::db::Database;
using vqro::rpc::VaqueroStorageServiceImpl;
using vqro::rpc::VaqueroSearchServiceImpl;


static grpc::Server* main_server = nullptr;
static mutex main_server_mutex;


void Shutdown() {  // Must be idempotent
  lock_guard<mutex> lck(main_server_mutex);
  if (main_server != nullptr) {
    LOG(INFO) << "stopping grpc server";
    main_server->Shutdown();
    main_server = nullptr;
  }
}


void HandleSignals(sigset_t* sigset) {
  int sig;
  while (true) {
    sigwait(sigset, &sig);

    switch (sig) {
      case SIGTERM:
        LOG(INFO) << "Caught SIGTERM, shutting down.";
        Shutdown();
        break;

      default:
        VLOG(2) << "Ignoring unhandled signal " << sig;
        break;
    }
  }
}


void RunServer(string server_address) {
  Database db(FLAGS_data_directory);
  VaqueroStorageServiceImpl storage_service(&db);
  VaqueroSearchServiceImpl search_service(db.search_engine.get());

  LOG(INFO) << "Working directory: " << vqro::GetCurrentWorkingDirectory();
  LOG(INFO) << "Database initialized in data_directory=" << FLAGS_data_directory;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&storage_service);
  builder.RegisterService(&search_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  {
    lock_guard<mutex> lck(main_server_mutex);
    main_server = server.get();
  }
  try {
    server->Wait();
  } catch (...) {
    lock_guard<mutex> lck(main_server_mutex);
    main_server = nullptr;
    LOG(ERROR) << "grpc::Server::Wait() threw an exception";
    throw;
  }
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

  // Validate flags
  if (FLAGS_data_directory.empty()) {
    std::cerr << "You must specify --data_directory" << std::endl;
    return 1;
  }

  // Setup signal handling. All threads share a common signal disposition, so
  // that disposition will be to block all signals, and one thread will
  // explicitly wait on incoming signals as they become pending. This ensures
  // random threads do not get interrupted by signals.
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGHUP);
  sigaddset(&sigset, SIGPIPE);
  sigaddset(&sigset, SIGTERM);
  if (pthread_sigmask(SIG_BLOCK, &sigset, NULL)) {
    PLOG(FATAL) << "pthread_sigmask failed";
  }
  std::thread sighandler(HandleSignals, &sigset);
  sighandler.detach();

  string server_address(FLAGS_listen_ip + ":" + to_string(FLAGS_listen_port));
  int ret = 0;

  grpc_init();
  try {
    RunServer(server_address);
  } catch (vqro::Error& error) {
    LOG(ERROR) << "Failed to start server: " << error.message;
    std::cerr << "Failed to start server: " << error.message << std::endl;
    ret = 1;
  }
  grpc_shutdown();  // This crashes sometimes...

  return ret;
}
