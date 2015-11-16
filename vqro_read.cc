#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <sstream>
#include <functional>

#include <jsoncpp/json/json.h>
#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <re2/re2.h>

#include "vqro/base/base.h"
#include "vqro/rpc/core.pb.h"
#include "vqro/rpc/search.grpc.pb.h"
#include "vqro/rpc/storage.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using vqro::rpc::VaqueroStorage;
using vqro::rpc::VaqueroSearch;
using vqro::rpc::Series;
using vqro::rpc::Datapoint;
using vqro::rpc::ReadOperation;
using vqro::rpc::ReadResult;
using vqro::rpc::SeriesQuery;
using vqro::rpc::LabelsQuery;
using vqro::rpc::LabelConstraint;
using vqro::rpc::SearchSeriesResults;
using vqro::rpc::SearchLabelsResults;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;

constexpr int64_t ONE_BILLION = 1000000000; // Don't forget the evil laugh


DEFINE_string(server_ip, "127.0.0.1", "Server IP to connect to");
DEFINE_int32(server_port, 7950, "Server port to connect to");
DEFINE_bool(h, false, "Print help on important flags");
DEFINE_string(start, "-5m", "Start of time range to read datapoints for."); //TODO more doc
DEFINE_string(end, "now", "End of time range ot read datapoints for.");
DEFINE_string(tick_unit, "", "A tick is the finest precision time unit vaquero can store. "
                              "... by setting the VQRO_TICK_UNIT environment variable.");
DEFINE_int32(search_limit, 0, "Limit how many series/labels can match the given query.");
DEFINE_int32(search_offset, 0, "Skip the first N matching series/labels, requires use of "
                               "--search_limit also.");
DEFINE_int32(datapoint_limit, 0, "Limit how many datapoints are read for each matching series.");
DEFINE_bool(prefer_latest, false, "If true you get the last N datapoints in the "
                                  "given time range, otherwise you get the first "
                                  "N datapoints, where N is defined by --datapoint_limit.");
DEFINE_bool(debug, false, "When true, print additional debug output to stderr.");
DEFINE_bool(json, false, "When true output is printed in JSON format, otherwise "
            "in a more human readable form.");
DEFINE_bool(search_labels, false, "If true, perform a SearchLabels call instead "
            "of ReadDatapoints.");
DEFINE_bool(search_series, false, "If true, perform a SearchSeries call instead "
            "of ReadDatapoints.");


const string kUsage(R"(
  vqro-read [flags] <label-query>

Valid tick_units: ns, us, ms, s, m, h, d

Absolute time formats:
  YYYY-mm-dd
  YYYY-mm-dd_HH:MM
  YYYY-mm-dd_HH:MM:SS
)");


class VaqueroClient {
 public:
  VaqueroClient(std::shared_ptr<Channel> channel)
      : storage_stub(VaqueroStorage::NewStub(channel)),
        search_stub(VaqueroSearch::NewStub(channel)) {}

  bool ReadDatapoints(const SeriesQuery& query,
                      int64_t start_time,
                      int64_t end_time,
                      int64_t datapoint_limit,
                      bool prefer_latest,
                      std::function<void(const ReadResult&)> callback)
  {
    ClientContext context;
    ReadResult result;;
    ReadOperation read_op;

    read_op.mutable_query()->CopyFrom(query);
    read_op.set_start_time(start_time);
    read_op.set_end_time(end_time);
    read_op.set_datapoint_limit(datapoint_limit);
    read_op.set_prefer_latest(prefer_latest);

    std::unique_ptr<ClientReader<ReadResult>> reader(
        storage_stub->ReadDatapoints(&context, read_op)
    );

    while (reader->Read(&result)) {
      callback(result);
    }

    Status status = reader->Finish();
    return status.ok();
  }

  bool SearchLabels(const LabelsQuery& query,
                    std::function<void(const SearchLabelsResults&)> callback)
  {
    ClientContext context;
    SearchLabelsResults results;
    std::unique_ptr<ClientReader<SearchLabelsResults>> reader(
        search_stub->SearchLabels(&context, query));

    while (reader->Read(&results))
      callback(results);

    Status status = reader->Finish();
    return status.ok();
  }

  bool SearchSeries(const SeriesQuery& query,
                    std::function<void(const SearchSeriesResults&)> callback)
  {
    ClientContext context;
    SearchSeriesResults results;
    std::unique_ptr<ClientReader<SearchSeriesResults>> reader(
        search_stub->SearchSeries(&context, query));

    while (reader->Read(&results))
      callback(results);

    Status status = reader->Finish();
    return status.ok();
  }

  void Shutdown() {
    storage_stub.reset();
    search_stub.reset();
  }

 private:
  std::unique_ptr<VaqueroStorage::Stub> storage_stub;
  std::unique_ptr<VaqueroSearch::Stub> search_stub;
};


enum class TickUnit {none, ns, us, ms, s, m, h, d};
TickUnit tick_unit = TickUnit::s;

std::map<string, TickUnit> TickUnitByString = {
  {"", TickUnit::none},
  {"ns", TickUnit::ns},
  {"us", TickUnit::us},
  {"ms", TickUnit::ms},
  {"s", TickUnit::s},
  {"m", TickUnit::m},
  {"h", TickUnit::h},
  {"d", TickUnit::d},
};

std::map<TickUnit, string> StringByTickUnit = {
  {TickUnit::none, ""},
  {TickUnit::ns, "ns"},
  {TickUnit::us, "us"},
  {TickUnit::ms, "ms"},
  {TickUnit::s, "s"},
  {TickUnit::m, "m"},
  {TickUnit::h, "h"},
  {TickUnit::d, "d"},
};


int64_t NanosecondsPerTick(TickUnit t) {
  switch (t) {
    case TickUnit::none: return 0; break;
    case TickUnit::ns: return 1; break;
    case TickUnit::us: return 1000; break;
    case TickUnit::ms: return 1000000; break;
    case TickUnit::s: return ONE_BILLION; break;
    case TickUnit::m: return ONE_BILLION * 60; break;
    case TickUnit::h: return ONE_BILLION * 3600; break;
    case TickUnit::d: return ONE_BILLION * 86400; break;
    default:
      return 0;
      break;
  }
  return 0; // not reached
}


void PrintUsage(string error) {
  google::ShowUsageWithFlagsRestrict(
      google::ProgramInvocationShortName(), __FILE__);
  cout << "\n\n" << error << "\n";
}


string FormatLabels(const Series& series) {
  std::ostringstream os;
  int commas = series.labels().size();
  os << "{";
  for (auto it : series.labels()) {
    os << it.first << "=" << it.second;
    if (--commas) os << ", ";
  }
  os << "}";
  return os.str();
}


void PrintReadResultJson(const ReadResult& result) {
  Json::Value obj;
  Json::Value labels_obj;
  Json::Value datapoints(Json::arrayValue);

  for (auto it : result.series().labels()) {
    labels_obj[it.first] = it.second;
  }
  obj["labels"] = labels_obj;

  for (auto p : result.datapoints()) {
    Json::Value point;
    Json::Int64 timestamp = p.timestamp();
    Json::Int64 duration = p.duration();
    point.append(timestamp);
    point.append(duration);
    point.append(p.value());
    datapoints.append(point);
  }
  obj["datapoints"] = datapoints;

  if (result.has_status()) {
    Json::Value status_obj;
    status_obj["text"] = result.status().text();
    status_obj["error"] = result.status().error();
    status_obj["go_away"] = result.status().go_away();
    obj["status"] = status_obj;
  }

  cout << obj << endl;
}


void PrintReadResult(const ReadResult& result) {
  cout << "ReadResult: labels=" << FormatLabels(result.series()) << endl;
  for (auto p : result.datapoints()) {
    cout << "    timestamp=" << p.timestamp()
         << "  duration=" << p.duration()
         << "  value=" << p.value() << endl;
  }
  if (result.has_status())
    cout << "StatusMessage: " << result.status().text()
         << " error=" << result.status().error() << endl;
  cout << endl;
}


void PrintSearchLabelsResults(const SearchLabelsResults& results) {
  if (FLAGS_json) {
    Json::Value list;
    for (auto label : results.labels()) {
      list.append(label);
    }
    cout << list << endl;
  } else { // non-json
    for (auto label : results.labels()) {
      cout << label << endl;
    }
    if (results.has_status())
      cout << "StatusMessage: " << results.status().text()
           << " error=" << results.status().error() << endl;
  }
}


void PrintSearchSeriesResults(const SearchSeriesResults& results) {
  if (FLAGS_json) {
    Json::Value list;
    for (auto series : results.matches()) {
      Json::Value obj;
      for (auto label : series.labels()) {
        obj[label.first] = label.second;
      }
      list.append(obj);
    }
    Json::FastWriter writer;
    cout << writer.write(list);
  } else { // non-json
    for (auto series : results.matches()) {
      cout << FormatLabels(series) << endl;
    }
    if (results.has_status())
      cout << "StatusMessage: " << results.status().text()
           << " error=" << results.status().error() << endl;
  }
}


int64_t Now() {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  int64_t now_in_ns = (now.tv_sec * ONE_BILLION) + now.tv_nsec;
  int64_t ns_per_tick = NanosecondsPerTick(tick_unit);
  if (ns_per_tick) {
    return now_in_ns / ns_per_tick;
  } else {
    return 0;
  }
}


int64_t GetTimeInTicks(string time_str, bool& relative) {
  if (time_str.empty() || time_str == "now")
    return Now();

  if (time_str == "min")
    return INT64_MIN;

  if (time_str == "max")
    return INT64_MAX;

  if (time_str[0] == '-' || time_str[0] == '+') { // Relative time specification
    relative = true;
    if (time_str[0] == '+')
      time_str.erase(0, 1);

    string digits;
    string unit_suffix;
    if (!RE2::FullMatch(time_str,
                        R"((-?\d+)([a-zA-Z]+)?)",
                        &digits, &unit_suffix))
      throw string("Failed to parse \"" + time_str + "\"");

    errno = 0;
    int64_t number = strtoll(digits.c_str(), NULL, 10);
    if (errno)
      throw ("Error parsing digits of time string \"" + time_str + "\": "
           + strerror(errno));

    TickUnit relative_unit = TickUnit::none;
    if (TickUnitByString.find(unit_suffix) != TickUnitByString.end()) {
      relative_unit = TickUnitByString.at(unit_suffix);
    } else if (!unit_suffix.empty()) {
      throw string("Error: invalid time unit suffix: " + time_str);
    }

    int64_t ns_per_tick = NanosecondsPerTick(tick_unit);
    if (!ns_per_tick)
      throw string("Error: Cannot specify relative time unit when no tick_unit is specified.");

    int64_t unit_ratio = NanosecondsPerTick(relative_unit) / ns_per_tick;
    return unit_ratio * number;

  } else { // Absolute time specification
    relative = false;

    if (tick_unit == TickUnit::none)
      throw string("Error: Cannot specify a date time without specifying a tick_unit");

    std::tm tm {};  // value initialize to zeroes
    tm.tm_isdst = -1;

    if (strptime(time_str.c_str(), "%Y-%m-%d_%H:%M:%S", &tm) == NULL)
      if (strptime(time_str.c_str(), "%Y-%m-%d_%H:%M", &tm) == NULL)
        if (strptime(time_str.c_str(), "%Y-%m-%d", &tm) == NULL)
          throw string("Unrecognized date time: " + time_str);

    time_t t = mktime(&tm);
    if (t == -1)
      throw string("mktime() error: cannot represent as epoch time");

    return t * NanosecondsPerTick(TickUnit::s) / NanosecondsPerTick(tick_unit);
  }
}


SeriesQuery BuildSeriesQuery(int argc, char** argv) {
  SeriesQuery query;
  if (FLAGS_search_limit) {
    query.set_result_limit(FLAGS_search_limit);
    if (FLAGS_search_offset)
      query.set_result_offset(FLAGS_search_offset);
  }

  LabelConstraint* con;
  string label_name;
  string predicate;
  for (int i = 1; i < argc; i++) {
    string arg(argv[i]);
    con = query.add_constraints();

    //TODO This is fugly.
    if (RE2::FullMatch(arg, R"(([^=]+)=~(.+))",
                       &label_name,
                       &predicate))
    {
      con->set_regex(predicate);
    } else if (RE2::FullMatch(arg, R"(([^=]+)=(.+))",
               &label_name,
               &predicate))
    {
      con->set_exact_value(predicate);
    } else {
      throw "Failed to parse label constraint \"" + arg + "\"";
    }

    con->set_label_name(label_name);
  }
  return query;
}


string GetServerAddress() {
  return FLAGS_server_ip + ":" + to_string(FLAGS_server_port);
}


int DoSearchLabels(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage("Insufficient arguments");
    return 1;
  }

  LabelsQuery query;
  query.set_regex(string(argv[1]));

  if (FLAGS_search_limit) {
    query.set_result_limit(FLAGS_search_limit);
    if (FLAGS_search_offset)
      query.set_result_offset(FLAGS_search_offset);
  }

  // RPC time
  grpc_init();
  auto channel = grpc::CreateChannel(GetServerAddress(),
                                     grpc::InsecureCredentials());
  VaqueroClient client(channel);

  if (FLAGS_debug)
    cerr << "SearchLabels() regex=" << query.regex() << endl;

  client.SearchLabels(query,
                      PrintSearchLabelsResults);

  client.Shutdown();
  grpc_shutdown();
  return 0;
}


int DoSearchSeries(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage("Insufficient arguments");
    return 1;
  }

  SeriesQuery query;
  try {
    query = BuildSeriesQuery(argc, argv);
  } catch (string& err) {
    cerr << err << endl;
    return 1;
  }


  // RPC time
  grpc_init();
  auto channel = grpc::CreateChannel(GetServerAddress(),
                                     grpc::InsecureCredentials());
  VaqueroClient client(channel);

  if (FLAGS_debug)
    cerr << "SearchSeries()\n";

  client.SearchSeries(query,
                      PrintSearchSeriesResults);

  client.Shutdown();
  grpc_shutdown();
  return 0;
}


int DoReadDatapoints(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage("Insufficient arguments");
    return 1;
  }

  SeriesQuery query;
  try {
    query = BuildSeriesQuery(argc, argv);
  } catch (string& err) {
    cerr << err << endl;
    return 1;
  }

  // Set tick_unit appropriately
  string env_tick_unit = vqro::GetEnvVar("VQRO_TICK_UNIT");
  if (!env_tick_unit.empty()) {
    if (TickUnitByString.find(env_tick_unit) == TickUnitByString.end()) {
      PrintUsage("Invalid tick_unit specified in VQRO_TICK_UNIT env var\n");
      return 1;
    }
    tick_unit = TickUnitByString.at(env_tick_unit);
  }

  if (!FLAGS_tick_unit.empty()) {
    if (TickUnitByString.find(FLAGS_tick_unit) == TickUnitByString.end()) {
      PrintUsage("Invalid tick_unit specified in --tick_unit flag\n");
      return 1;
    }
    tick_unit = TickUnitByString.at(FLAGS_tick_unit);
  }


  int64_t start_time;
  int64_t end_time;
  bool start_is_relative = false;
  bool end_is_relative = false;

  try {
    start_time = GetTimeInTicks(FLAGS_start, start_is_relative);
  } catch (string error_message) {
    PrintUsage("Failed to parse --start flag: " + error_message);
    return 1;
  }

  try {
    end_time = GetTimeInTicks(FLAGS_end, end_is_relative);
  } catch (string error_message) {
    PrintUsage("Failed to parse --end flag: " + error_message);
    return 1;
  }

  // If either --start or --end is relative, compute absolute timestamp for it.
  if (start_is_relative && end_is_relative) {
    cerr << "Either --start or --end may be relative, but not both.\n";
    return 1;
  }

  if (start_is_relative) {
    start_time += end_time;
  } else if (end_is_relative) {
    end_time += start_time;
  }

  if (start_time >= end_time) {
    cerr << "Invalid time range, start=" << start_time
         << " end=" << end_time << endl;
    return 1;
  }

  // RPC time
  grpc_init();
  auto channel = grpc::CreateChannel(GetServerAddress(),
                                     grpc::InsecureCredentials());
  VaqueroClient client(channel);

  if (FLAGS_debug)
    cerr << "ReadDatapoints() start=" << start_time << " end=" << end_time << endl;

  client.ReadDatapoints(query,
                        start_time,
                        end_time,
                        FLAGS_datapoint_limit,
                        FLAGS_prefer_latest,
                        (FLAGS_json) ? PrintReadResultJson : PrintReadResult);

  client.Shutdown();
  grpc_shutdown();
  return 0;
}


int main(int argc, char** argv) {
  google::SetUsageMessage(kUsage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_h) {
    PrintUsage("");
    return 0;
  }

  if (FLAGS_search_labels) {
    return DoSearchLabels(argc, argv);
  } else if (FLAGS_search_series) {
    return DoSearchSeries(argc, argv);
  } else {
    return DoReadDatapoints(argc, argv);
  }
}
