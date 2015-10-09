#ifndef VQRO_RPC_SEARCH_H
#define VQRO_RPC_SEARCH_H

#include <glog/logging.h>
#include "vqro/rpc/search.grpc.pb.h"
#include "vqro/db/search_engine.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerWriter;


namespace vqro {
namespace rpc {


class VaqueroSearchServiceImpl final : public VaqueroSearch::Service {
 public:
  VaqueroSearchServiceImpl(vqro::db::SearchEngine* const se)
      : search_engine(se) {}

 private:
  vqro::db::SearchEngine* const search_engine;

  Status SearchSeries(ServerContext* context,
                      const SeriesQuery* query,
                      ServerWriter<SearchSeriesResults>* writer) override {

    auto respond = [&] (SearchSeriesResults& results) { writer->Write(results); };
    try {
      search_engine->SearchSeries(*query, respond);
    } catch (vqro::db::SqliteError& err) {
      LOG(ERROR) << "SqliteError during SearchSeries: " << err.message; //XXX StatusMessage
    }
    return Status::OK;
  }


  Status SearchLabels(ServerContext* context,
                      const LabelsQuery* query,
                      ServerWriter<SearchLabelsResults>* writer) override {
    
    auto respond = [&] (SearchLabelsResults& results) { writer->Write(results); };
    try {
      search_engine->SearchLabels(*query, respond);
    } catch (vqro::db::SqliteError& err) {
      LOG(ERROR) << "SqliteError during SearchLabels: " << err.message; //XXX StatusMessage
    }
    return Status::OK;
  }
};


} // namespace rpc
} // namespace vqro

#endif // VQRO_RPC_SEARCH_H
