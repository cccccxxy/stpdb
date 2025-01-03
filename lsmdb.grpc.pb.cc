// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: lsmdb.proto

#include "lsmdb.pb.h"
#include "lsmdb.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace lsmdb {
namespace v1 {

static const char* Lsmdb_method_names[] = {
  "/lsmdb.v1.Lsmdb/OpenDB",
  "/lsmdb.v1.Lsmdb/OpenDBWeb",
  "/lsmdb.v1.Lsmdb/Put",
  "/lsmdb.v1.Lsmdb/BatchPut",
  "/lsmdb.v1.Lsmdb/PutStr",
  "/lsmdb.v1.Lsmdb/Get",
  "/lsmdb.v1.Lsmdb/CloseDB",
  "/lsmdb.v1.Lsmdb/CloseDBWeb",
  "/lsmdb.v1.Lsmdb/TransferKV",
  "/lsmdb.v1.Lsmdb/Transfer",
  "/lsmdb.v1.Lsmdb/GetKVs",
  "/lsmdb.v1.Lsmdb/PrefixData",
};

std::unique_ptr< Lsmdb::Stub> Lsmdb::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< Lsmdb::Stub> stub(new Lsmdb::Stub(channel, options));
  return stub;
}

Lsmdb::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_OpenDB_(Lsmdb_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OpenDBWeb_(Lsmdb_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Put_(Lsmdb_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_BatchPut_(Lsmdb_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_PutStr_(Lsmdb_method_names[4], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Get_(Lsmdb_method_names[5], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_CloseDB_(Lsmdb_method_names[6], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_CloseDBWeb_(Lsmdb_method_names[7], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_TransferKV_(Lsmdb_method_names[8], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Transfer_(Lsmdb_method_names[9], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetKVs_(Lsmdb_method_names[10], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_PrefixData_(Lsmdb_method_names[11], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Lsmdb::Stub::OpenDB(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBRequest& request, ::lsmdb::v1::OpenDBReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::OpenDBRequest, ::lsmdb::v1::OpenDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_OpenDB_, context, request, response);
}

void Lsmdb::Stub::async::OpenDB(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBRequest* request, ::lsmdb::v1::OpenDBReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::OpenDBRequest, ::lsmdb::v1::OpenDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_OpenDB_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::OpenDB(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBRequest* request, ::lsmdb::v1::OpenDBReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_OpenDB_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::OpenDBReply>* Lsmdb::Stub::PrepareAsyncOpenDBRaw(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::OpenDBReply, ::lsmdb::v1::OpenDBRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_OpenDB_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::OpenDBReply>* Lsmdb::Stub::AsyncOpenDBRaw(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncOpenDBRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::OpenDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBWebRequest& request, ::lsmdb::v1::OpenDBWebReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::OpenDBWebRequest, ::lsmdb::v1::OpenDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_OpenDBWeb_, context, request, response);
}

void Lsmdb::Stub::async::OpenDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBWebRequest* request, ::lsmdb::v1::OpenDBWebReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::OpenDBWebRequest, ::lsmdb::v1::OpenDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_OpenDBWeb_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::OpenDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBWebRequest* request, ::lsmdb::v1::OpenDBWebReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_OpenDBWeb_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::OpenDBWebReply>* Lsmdb::Stub::PrepareAsyncOpenDBWebRaw(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBWebRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::OpenDBWebReply, ::lsmdb::v1::OpenDBWebRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_OpenDBWeb_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::OpenDBWebReply>* Lsmdb::Stub::AsyncOpenDBWebRaw(::grpc::ClientContext* context, const ::lsmdb::v1::OpenDBWebRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncOpenDBWebRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::Put(::grpc::ClientContext* context, const ::lsmdb::v1::PutRequest& request, ::lsmdb::v1::PutReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::PutRequest, ::lsmdb::v1::PutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Put_, context, request, response);
}

void Lsmdb::Stub::async::Put(::grpc::ClientContext* context, const ::lsmdb::v1::PutRequest* request, ::lsmdb::v1::PutReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::PutRequest, ::lsmdb::v1::PutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Put_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::Put(::grpc::ClientContext* context, const ::lsmdb::v1::PutRequest* request, ::lsmdb::v1::PutReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Put_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PutReply>* Lsmdb::Stub::PrepareAsyncPutRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PutRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::PutReply, ::lsmdb::v1::PutRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Put_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PutReply>* Lsmdb::Stub::AsyncPutRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PutRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncPutRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::BatchPut(::grpc::ClientContext* context, const ::lsmdb::v1::BatchPutRequest& request, ::lsmdb::v1::BatchPutReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::BatchPutRequest, ::lsmdb::v1::BatchPutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_BatchPut_, context, request, response);
}

void Lsmdb::Stub::async::BatchPut(::grpc::ClientContext* context, const ::lsmdb::v1::BatchPutRequest* request, ::lsmdb::v1::BatchPutReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::BatchPutRequest, ::lsmdb::v1::BatchPutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_BatchPut_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::BatchPut(::grpc::ClientContext* context, const ::lsmdb::v1::BatchPutRequest* request, ::lsmdb::v1::BatchPutReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_BatchPut_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::BatchPutReply>* Lsmdb::Stub::PrepareAsyncBatchPutRaw(::grpc::ClientContext* context, const ::lsmdb::v1::BatchPutRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::BatchPutReply, ::lsmdb::v1::BatchPutRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_BatchPut_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::BatchPutReply>* Lsmdb::Stub::AsyncBatchPutRaw(::grpc::ClientContext* context, const ::lsmdb::v1::BatchPutRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncBatchPutRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::PutStr(::grpc::ClientContext* context, const ::lsmdb::v1::PutStrRequest& request, ::lsmdb::v1::PutStrReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::PutStrRequest, ::lsmdb::v1::PutStrReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_PutStr_, context, request, response);
}

void Lsmdb::Stub::async::PutStr(::grpc::ClientContext* context, const ::lsmdb::v1::PutStrRequest* request, ::lsmdb::v1::PutStrReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::PutStrRequest, ::lsmdb::v1::PutStrReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PutStr_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::PutStr(::grpc::ClientContext* context, const ::lsmdb::v1::PutStrRequest* request, ::lsmdb::v1::PutStrReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PutStr_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PutStrReply>* Lsmdb::Stub::PrepareAsyncPutStrRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PutStrRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::PutStrReply, ::lsmdb::v1::PutStrRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_PutStr_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PutStrReply>* Lsmdb::Stub::AsyncPutStrRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PutStrRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncPutStrRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::Get(::grpc::ClientContext* context, const ::lsmdb::v1::GetRequest& request, ::lsmdb::v1::GetReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::GetRequest, ::lsmdb::v1::GetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Get_, context, request, response);
}

void Lsmdb::Stub::async::Get(::grpc::ClientContext* context, const ::lsmdb::v1::GetRequest* request, ::lsmdb::v1::GetReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::GetRequest, ::lsmdb::v1::GetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::Get(::grpc::ClientContext* context, const ::lsmdb::v1::GetRequest* request, ::lsmdb::v1::GetReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::GetReply>* Lsmdb::Stub::PrepareAsyncGetRaw(::grpc::ClientContext* context, const ::lsmdb::v1::GetRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::GetReply, ::lsmdb::v1::GetRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Get_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::GetReply>* Lsmdb::Stub::AsyncGetRaw(::grpc::ClientContext* context, const ::lsmdb::v1::GetRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::CloseDB(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBRequest& request, ::lsmdb::v1::CloseDBReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::CloseDBRequest, ::lsmdb::v1::CloseDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_CloseDB_, context, request, response);
}

void Lsmdb::Stub::async::CloseDB(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBRequest* request, ::lsmdb::v1::CloseDBReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::CloseDBRequest, ::lsmdb::v1::CloseDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CloseDB_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::CloseDB(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBRequest* request, ::lsmdb::v1::CloseDBReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CloseDB_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::CloseDBReply>* Lsmdb::Stub::PrepareAsyncCloseDBRaw(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::CloseDBReply, ::lsmdb::v1::CloseDBRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_CloseDB_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::CloseDBReply>* Lsmdb::Stub::AsyncCloseDBRaw(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCloseDBRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::CloseDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBWebRequest& request, ::lsmdb::v1::CloseDBWebReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::CloseDBWebRequest, ::lsmdb::v1::CloseDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_CloseDBWeb_, context, request, response);
}

void Lsmdb::Stub::async::CloseDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBWebRequest* request, ::lsmdb::v1::CloseDBWebReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::CloseDBWebRequest, ::lsmdb::v1::CloseDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CloseDBWeb_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::CloseDBWeb(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBWebRequest* request, ::lsmdb::v1::CloseDBWebReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CloseDBWeb_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::CloseDBWebReply>* Lsmdb::Stub::PrepareAsyncCloseDBWebRaw(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBWebRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::CloseDBWebReply, ::lsmdb::v1::CloseDBWebRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_CloseDBWeb_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::CloseDBWebReply>* Lsmdb::Stub::AsyncCloseDBWebRaw(::grpc::ClientContext* context, const ::lsmdb::v1::CloseDBWebRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCloseDBWebRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::TransferKV(::grpc::ClientContext* context, const ::lsmdb::v1::TransferKVRequest& request, ::lsmdb::v1::TransferKVReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::TransferKVRequest, ::lsmdb::v1::TransferKVReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_TransferKV_, context, request, response);
}

void Lsmdb::Stub::async::TransferKV(::grpc::ClientContext* context, const ::lsmdb::v1::TransferKVRequest* request, ::lsmdb::v1::TransferKVReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::TransferKVRequest, ::lsmdb::v1::TransferKVReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_TransferKV_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::TransferKV(::grpc::ClientContext* context, const ::lsmdb::v1::TransferKVRequest* request, ::lsmdb::v1::TransferKVReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_TransferKV_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::TransferKVReply>* Lsmdb::Stub::PrepareAsyncTransferKVRaw(::grpc::ClientContext* context, const ::lsmdb::v1::TransferKVRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::TransferKVReply, ::lsmdb::v1::TransferKVRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_TransferKV_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::TransferKVReply>* Lsmdb::Stub::AsyncTransferKVRaw(::grpc::ClientContext* context, const ::lsmdb::v1::TransferKVRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncTransferKVRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::Transfer(::grpc::ClientContext* context, const ::lsmdb::v1::TransferRequest& request, ::lsmdb::v1::TransferReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::TransferRequest, ::lsmdb::v1::TransferReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Transfer_, context, request, response);
}

void Lsmdb::Stub::async::Transfer(::grpc::ClientContext* context, const ::lsmdb::v1::TransferRequest* request, ::lsmdb::v1::TransferReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::TransferRequest, ::lsmdb::v1::TransferReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Transfer_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::Transfer(::grpc::ClientContext* context, const ::lsmdb::v1::TransferRequest* request, ::lsmdb::v1::TransferReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Transfer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::TransferReply>* Lsmdb::Stub::PrepareAsyncTransferRaw(::grpc::ClientContext* context, const ::lsmdb::v1::TransferRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::TransferReply, ::lsmdb::v1::TransferRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Transfer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::TransferReply>* Lsmdb::Stub::AsyncTransferRaw(::grpc::ClientContext* context, const ::lsmdb::v1::TransferRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncTransferRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::GetKVs(::grpc::ClientContext* context, const ::lsmdb::v1::GetKVsRequest& request, ::lsmdb::v1::GetKVsReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::GetKVsRequest, ::lsmdb::v1::GetKVsReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetKVs_, context, request, response);
}

void Lsmdb::Stub::async::GetKVs(::grpc::ClientContext* context, const ::lsmdb::v1::GetKVsRequest* request, ::lsmdb::v1::GetKVsReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::GetKVsRequest, ::lsmdb::v1::GetKVsReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVs_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::GetKVs(::grpc::ClientContext* context, const ::lsmdb::v1::GetKVsRequest* request, ::lsmdb::v1::GetKVsReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVs_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::GetKVsReply>* Lsmdb::Stub::PrepareAsyncGetKVsRaw(::grpc::ClientContext* context, const ::lsmdb::v1::GetKVsRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::GetKVsReply, ::lsmdb::v1::GetKVsRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetKVs_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::GetKVsReply>* Lsmdb::Stub::AsyncGetKVsRaw(::grpc::ClientContext* context, const ::lsmdb::v1::GetKVsRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetKVsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Lsmdb::Stub::PrefixData(::grpc::ClientContext* context, const ::lsmdb::v1::PrefixRequest& request, ::lsmdb::v1::PrefixResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::lsmdb::v1::PrefixRequest, ::lsmdb::v1::PrefixResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_PrefixData_, context, request, response);
}

void Lsmdb::Stub::async::PrefixData(::grpc::ClientContext* context, const ::lsmdb::v1::PrefixRequest* request, ::lsmdb::v1::PrefixResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::lsmdb::v1::PrefixRequest, ::lsmdb::v1::PrefixResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PrefixData_, context, request, response, std::move(f));
}

void Lsmdb::Stub::async::PrefixData(::grpc::ClientContext* context, const ::lsmdb::v1::PrefixRequest* request, ::lsmdb::v1::PrefixResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PrefixData_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PrefixResponse>* Lsmdb::Stub::PrepareAsyncPrefixDataRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PrefixRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::lsmdb::v1::PrefixResponse, ::lsmdb::v1::PrefixRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_PrefixData_, context, request);
}

::grpc::ClientAsyncResponseReader< ::lsmdb::v1::PrefixResponse>* Lsmdb::Stub::AsyncPrefixDataRaw(::grpc::ClientContext* context, const ::lsmdb::v1::PrefixRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncPrefixDataRaw(context, request, cq);
  result->StartCall();
  return result;
}

Lsmdb::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::OpenDBRequest, ::lsmdb::v1::OpenDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::OpenDBRequest* req,
             ::lsmdb::v1::OpenDBReply* resp) {
               return service->OpenDB(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::OpenDBWebRequest, ::lsmdb::v1::OpenDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::OpenDBWebRequest* req,
             ::lsmdb::v1::OpenDBWebReply* resp) {
               return service->OpenDBWeb(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::PutRequest, ::lsmdb::v1::PutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::PutRequest* req,
             ::lsmdb::v1::PutReply* resp) {
               return service->Put(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::BatchPutRequest, ::lsmdb::v1::BatchPutReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::BatchPutRequest* req,
             ::lsmdb::v1::BatchPutReply* resp) {
               return service->BatchPut(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[4],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::PutStrRequest, ::lsmdb::v1::PutStrReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::PutStrRequest* req,
             ::lsmdb::v1::PutStrReply* resp) {
               return service->PutStr(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[5],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::GetRequest, ::lsmdb::v1::GetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::GetRequest* req,
             ::lsmdb::v1::GetReply* resp) {
               return service->Get(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[6],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::CloseDBRequest, ::lsmdb::v1::CloseDBReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::CloseDBRequest* req,
             ::lsmdb::v1::CloseDBReply* resp) {
               return service->CloseDB(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[7],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::CloseDBWebRequest, ::lsmdb::v1::CloseDBWebReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::CloseDBWebRequest* req,
             ::lsmdb::v1::CloseDBWebReply* resp) {
               return service->CloseDBWeb(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[8],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::TransferKVRequest, ::lsmdb::v1::TransferKVReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::TransferKVRequest* req,
             ::lsmdb::v1::TransferKVReply* resp) {
               return service->TransferKV(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[9],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::TransferRequest, ::lsmdb::v1::TransferReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::TransferRequest* req,
             ::lsmdb::v1::TransferReply* resp) {
               return service->Transfer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[10],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::GetKVsRequest, ::lsmdb::v1::GetKVsReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::GetKVsRequest* req,
             ::lsmdb::v1::GetKVsReply* resp) {
               return service->GetKVs(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Lsmdb_method_names[11],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Lsmdb::Service, ::lsmdb::v1::PrefixRequest, ::lsmdb::v1::PrefixResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Lsmdb::Service* service,
             ::grpc::ServerContext* ctx,
             const ::lsmdb::v1::PrefixRequest* req,
             ::lsmdb::v1::PrefixResponse* resp) {
               return service->PrefixData(ctx, req, resp);
             }, this)));
}

Lsmdb::Service::~Service() {
}

::grpc::Status Lsmdb::Service::OpenDB(::grpc::ServerContext* context, const ::lsmdb::v1::OpenDBRequest* request, ::lsmdb::v1::OpenDBReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::OpenDBWeb(::grpc::ServerContext* context, const ::lsmdb::v1::OpenDBWebRequest* request, ::lsmdb::v1::OpenDBWebReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::Put(::grpc::ServerContext* context, const ::lsmdb::v1::PutRequest* request, ::lsmdb::v1::PutReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::BatchPut(::grpc::ServerContext* context, const ::lsmdb::v1::BatchPutRequest* request, ::lsmdb::v1::BatchPutReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::PutStr(::grpc::ServerContext* context, const ::lsmdb::v1::PutStrRequest* request, ::lsmdb::v1::PutStrReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::Get(::grpc::ServerContext* context, const ::lsmdb::v1::GetRequest* request, ::lsmdb::v1::GetReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::CloseDB(::grpc::ServerContext* context, const ::lsmdb::v1::CloseDBRequest* request, ::lsmdb::v1::CloseDBReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::CloseDBWeb(::grpc::ServerContext* context, const ::lsmdb::v1::CloseDBWebRequest* request, ::lsmdb::v1::CloseDBWebReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::TransferKV(::grpc::ServerContext* context, const ::lsmdb::v1::TransferKVRequest* request, ::lsmdb::v1::TransferKVReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::Transfer(::grpc::ServerContext* context, const ::lsmdb::v1::TransferRequest* request, ::lsmdb::v1::TransferReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::GetKVs(::grpc::ServerContext* context, const ::lsmdb::v1::GetKVsRequest* request, ::lsmdb::v1::GetKVsReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Lsmdb::Service::PrefixData(::grpc::ServerContext* context, const ::lsmdb::v1::PrefixRequest* request, ::lsmdb::v1::PrefixResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace lsmdb
}  // namespace v1

