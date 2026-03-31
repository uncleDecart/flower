#include "grpc_rere.h"
#include "flwr/proto/fleet.grpc.pb.h"

gRPCRereCommunicator::gRPCRereCommunicator(std::string server_address,
                                           int grpc_max_message_length) {
  grpc::ChannelArguments args;
  args.SetMaxReceiveMessageSize(grpc_max_message_length);
  args.SetMaxSendMessageSize(grpc_max_message_length);

  // Establish an insecure gRPC connection to a gRPC server
  std::shared_ptr<grpc::Channel> channel = grpc::CreateCustomChannel(
      server_address, grpc::InsecureChannelCredentials(), args);

  // Create stub
  stub = flwr::proto::Fleet::NewStub(channel);
}

bool gRPCRereCommunicator::send_register_node(
    flwr::proto::RegisterNodeFleetRequest request,
    flwr::proto::RegisterNodeFleetResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->RegisterNode(&context, request, response);
  if (!status.ok()) {
    std::cerr << "RegisterNode RPC failed: " << status.error_message()
              << std::endl;
    return false;
  }

  return true;
}

bool gRPCRereCommunicator::send_activate_node(
    flwr::proto::ActivateNodeRequest request,
    flwr::proto::ActivateNodeResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->ActivateNode(&context, request, response);
  if (!status.ok()) {
    std::cerr << "ActivateNode RPC failed: " << status.error_message()
              << std::endl;
    return false;
  }

  return true;
}

bool gRPCRereCommunicator::send_deactivate_node(
    flwr::proto::DeactivateNodeRequest request,
    flwr::proto::DeactivateNodeResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->DeactivateNode(&context, request, response);

  if (!status.ok()) {
    std::cerr << "DeactivateNode RPC failed with status: " << status.error_message()
              << std::endl;
    return false;
  }

  return true;
}

bool gRPCRereCommunicator::send_unregister_node(
    flwr::proto::UnregisterNodeFleetRequest request,
    flwr::proto::UnregisterNodeFleetResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->UnregisterNode(&context, request, response);

  if (!status.ok()) {
    std::cerr << "UnregisterNode RPC failed with status: " << status.error_message()
              << std::endl;
    return false;
  }

  return true;
}

bool gRPCRereCommunicator::send_pull_messages(
    flwr::proto::PullMessagesRequest request,
    flwr::proto::PullMessagesResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->PullMessages(&context, request, response);

  if (!status.ok()) {
    std::cerr << "PullMessages RPC failed with status: "
              << status.error_message() << std::endl;
    return false;
  }

  return true;
}

bool gRPCRereCommunicator::send_push_messages(
    flwr::proto::PushMessagesRequest request,
    flwr::proto::PushMessagesResponse *response) {
  grpc::ClientContext context;
  grpc::Status status = stub->PushMessages(&context, request, response);

  if (!status.ok()) {
    std::cerr << "PushMessages RPC failed with status: "
              << status.error_message() << std::endl;
    return false;
  }

  return true;
}
