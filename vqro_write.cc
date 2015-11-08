#include <iostream>
#include <string>
#include <csignal>
#include <thread>

#include <jsoncpp/json/json.h>
#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

#include "vqro/base/base.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/storage.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;

using vqro::rpc::VaqueroStorage;
using vqro::rpc::Datapoint;
using vqro::rpc::WriteOperation;
using vqro::rpc::StatusMessage;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;


DEFINE_string(ip, "127.0.0.1", "Server IP to connect to");
DEFINE_int32(port, 7950, "Server port to connect to");
DEFINE_bool(h, false, "Print help on important flags");

bool running = true;

// The type of a literal 0 is ambiguous because it is both an int and a NULL pointer.
// We need to use zero as an index into a json array, so we disambiguate here.
Json::Value::ArrayIndex zero(0);


const string kUsage(R"(
  vqro-write [flags]

Datapoints are read on STDIN via newline-delimited json objects like this:

{"labels": {"foo": "bar"}, "datapoints": [[0, 10, 42], [10, 10, 3.14]]}

This specifies two datapoints with the labels {foo="bar"}. The first has
timestamp=0, duration=10, value=42.0. The second has timestamp=10, duration=10,
value=3.14. Note that newlines are not allowed within a json object.
)");


class VaqueroClient {
 public:
  VaqueroClient(std::shared_ptr<Channel> channel)
      : stub(VaqueroStorage::NewStub(channel)) {}

  inline void Write(WriteOperation& write_op)
  {
    if (stream == nullptr) {
      stream = stub->WriteDatapoints(&context);
      //std::thread status_reader([&] { ReadStatusMessages(); }); //TODO fix with newer grpc
      //status_reader.detach();
    }
    stream->Write(write_op);
  }

  void Shutdown() {
    if (stream != nullptr) {
      stream->WritesDone();
      stream->Finish();
    }
    stub.reset();
  }

 private:
  ClientContext context;
  StatusMessage status_msg;
  std::unique_ptr<VaqueroStorage::Stub> stub;
  std::unique_ptr<ClientReaderWriter<WriteOperation, StatusMessage>> stream;

  void ReadStatusMessages() {
    while (true) { // This is probably not the slightest bit thread-safe
      status_msg.Clear();
      stream->Read(&status_msg);
      cerr << "StatusMessage received: " << status_msg.text()
           << " (error=" << status_msg.error()
           << ", go_away=" << status_msg.go_away() << ")\n";

      if (status_msg.go_away())
        Shutdown();
    }
  }
};


void handle_sigint(int sig_num) {
  running = false;
  fclose(stdin);
}


int main(int argc, char** argv) {
  google::SetUsageMessage(kUsage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_h) {
    google::ShowUsageWithFlagsRestrict(
        google::ProgramInvocationShortName(), __FILE__);
    return 0;
  }

  string server_address(FLAGS_ip + ":" + to_string(FLAGS_port));

  grpc_init();
  signal(SIGINT, handle_sigint);
  auto channel = grpc::CreateChannel(server_address,
                                     grpc::InsecureCredentials());
  VaqueroClient client(channel);

  WriteOperation write_op;
  Datapoint* point;

  // Parse a stream of newline-delimited json objects from stdin
  string line;
  Json::Value record;
  Json::Value labels_val;
  Json::Value datapoints_val;
  Json::Value null_val;
  Json::Reader reader;
  int return_code = 0;

  cin.sync_with_stdio(false);
  while (running) {
    std::getline(cin, line);

    if (cin.eof())
      break;

    if (line.empty())
      continue;

    record.clear();
    if (!reader.parse(line, record, false)) {
      cerr << "Failed to parse JSON: " << line << endl;
      cerr << reader.getFormattedErrorMessages();
      return_code = 1;
      break;
    }

    labels_val = record["labels"];
    datapoints_val = record["datapoints"];

    if (!labels_val || !datapoints_val) {
      cerr << "JSON object invalid, 'labels' and 'datapoints' members required.\n";
      cerr << line << endl;
      return_code = 1;
      break;
    }

    write_op.Clear();

    // Copy the labels to the write_op protobuf
    auto labels = write_op.mutable_series()->mutable_labels();
    labels->clear();
    for (auto member : labels_val.getMemberNames()) {
      (*labels)[member] = labels_val.get(member, null_val).asString();
    }

    // Copy the datapoints to the write_op protobuf
    bool error = false;
    for (auto it = datapoints_val.begin(); it != datapoints_val.end(); it++) {
      Json::Value val = *it;
      if (val.size() != 3) {
        error = true;
        cerr << "Invalid datapoint, exactly 3 elements required: " << line << endl;
        break;
      }
      point = write_op.add_datapoints();
      point->set_timestamp(val.get(zero, null_val).asInt64());
      point->set_duration(val.get(1, null_val).asInt64());
      point->set_value(val.get(2, null_val).asDouble());
    }
    if (error) break;

    if (write_op.datapoints_size())
      client.Write(write_op);
  }

  client.Shutdown();
  grpc_shutdown();
  return return_code;
}
