// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: storage.proto
#ifndef GRPC_storage_2eproto__INCLUDED
#define GRPC_storage_2eproto__INCLUDED

#include "storage.pb.h"

#include <grpc++/support/async_stream.h>
#include <grpc++/impl/rpc_method.h>
#include <grpc++/impl/proto_utils.h>
#include <grpc++/impl/service_type.h>
#include <grpc++/support/async_unary_call.h>
#include <grpc++/support/status.h>
#include <grpc++/support/stub_options.h>
#include <grpc++/support/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class RpcService;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

namespace vqro {
namespace rpc {

class VaqueroStorage GRPC_FINAL {
 public:
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>> WriteDatapoints(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>>(WriteDatapointsRaw(context));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>> AsyncWriteDatapoints(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>>(AsyncWriteDatapointsRaw(context, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientReaderInterface< ::vqro::rpc::ReadResult>> ReadDatapoints(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request) {
      return std::unique_ptr< ::grpc::ClientReaderInterface< ::vqro::rpc::ReadResult>>(ReadDatapointsRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::vqro::rpc::ReadResult>> AsyncReadDatapoints(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::vqro::rpc::ReadResult>>(AsyncReadDatapointsRaw(context, request, cq, tag));
    }
  private:
    virtual ::grpc::ClientReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>* WriteDatapointsRaw(::grpc::ClientContext* context) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>* AsyncWriteDatapointsRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientReaderInterface< ::vqro::rpc::ReadResult>* ReadDatapointsRaw(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::vqro::rpc::ReadResult>* AsyncReadDatapointsRaw(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request, ::grpc::CompletionQueue* cq, void* tag) = 0;
  };
  class Stub GRPC_FINAL : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::Channel>& channel);
    std::unique_ptr< ::grpc::ClientReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>> WriteDatapoints(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>>(WriteDatapointsRaw(context));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>> AsyncWriteDatapoints(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>>(AsyncWriteDatapointsRaw(context, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientReader< ::vqro::rpc::ReadResult>> ReadDatapoints(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request) {
      return std::unique_ptr< ::grpc::ClientReader< ::vqro::rpc::ReadResult>>(ReadDatapointsRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::vqro::rpc::ReadResult>> AsyncReadDatapoints(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::vqro::rpc::ReadResult>>(AsyncReadDatapointsRaw(context, request, cq, tag));
    }

   private:
    std::shared_ptr< ::grpc::Channel> channel_;
    ::grpc::ClientReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>* WriteDatapointsRaw(::grpc::ClientContext* context) GRPC_OVERRIDE;
    ::grpc::ClientAsyncReaderWriter< ::vqro::rpc::WriteOperation, ::vqro::rpc::StatusMessage>* AsyncWriteDatapointsRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    ::grpc::ClientReader< ::vqro::rpc::ReadResult>* ReadDatapointsRaw(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request) GRPC_OVERRIDE;
    ::grpc::ClientAsyncReader< ::vqro::rpc::ReadResult>* AsyncReadDatapointsRaw(::grpc::ClientContext* context, const ::vqro::rpc::ReadOperation& request, ::grpc::CompletionQueue* cq, void* tag) GRPC_OVERRIDE;
    const ::grpc::RpcMethod rpcmethod_WriteDatapoints_;
    const ::grpc::RpcMethod rpcmethod_ReadDatapoints_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::Channel>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::SynchronousService {
   public:
    Service() : service_(nullptr) {}
    virtual ~Service();
    virtual ::grpc::Status WriteDatapoints(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::vqro::rpc::StatusMessage, ::vqro::rpc::WriteOperation>* stream);
    virtual ::grpc::Status ReadDatapoints(::grpc::ServerContext* context, const ::vqro::rpc::ReadOperation* request, ::grpc::ServerWriter< ::vqro::rpc::ReadResult>* writer);
    ::grpc::RpcService* service() GRPC_OVERRIDE GRPC_FINAL;
   private:
    ::grpc::RpcService* service_;
  };
  class AsyncService GRPC_FINAL : public ::grpc::AsynchronousService {
   public:
    explicit AsyncService();
    ~AsyncService() {};
    void RequestWriteDatapoints(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::vqro::rpc::StatusMessage, ::vqro::rpc::WriteOperation>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag);
    void RequestReadDatapoints(::grpc::ServerContext* context, ::vqro::rpc::ReadOperation* request, ::grpc::ServerAsyncWriter< ::vqro::rpc::ReadResult>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag);
  };
};

}  // namespace rpc
}  // namespace vqro


#endif  // GRPC_storage_2eproto__INCLUDED