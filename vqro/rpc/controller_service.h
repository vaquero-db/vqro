#ifndef VQRO_RPC_CONTROLLER_H
#define VQRO_RPC_CONTROLLER_H

#include <glog/logging.h>

#include "vqro/control/controller.h"
#include "vqro/rpc/controller.grpc.pb.h"
#include "vqro/rpc/search.grpc.pb.h"


using grpc::Status;
using grpc::ServerContext;
using vqro::control::Controller;


namespace vqro {
namespace rpc {


class VaqueroControllerServiceImpl final : public VaqueroController::Service {
 public:
  VaqueroControllerServiceImpl(Controller* c) : controller(c) {}

 private:
  Controller* const controller;

  Status LocateSeries(ServerContext* context,
                      const SeriesQuery* query,
                      LocateSeriesResults* results) override {
    LOG(INFO) << "LocateSeries() called";
    //XXX
    return Status::OK;
  }

  Status ExchangeState(ServerContext* context,
                       const ExchangeStateRequest* request,
                       ExchangeStateResponse* response) override {
    LOG(INFO) << "ExchangeState() called";
    //XXX
    return Status::OK;
  }
};


} // namespace rpc
} // namespace vqro

#endif // VQRO_RPC_CONTROLLER_H
