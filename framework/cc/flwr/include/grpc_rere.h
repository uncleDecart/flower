/*************************************************************************************************
 *
 * @file grpc-rere.h
 *
 * @brief Provide functions for establishing gRPC request-response communication
 *
 * @author The Flower Authors
 *
 * @version 1.0
 *
 * @date 06/11/2023
 *
 *************************************************************************************************/

#ifndef GRPC_RERE_H
#define GRPC_RERE_H
#pragma once
#include "communicator.h"
#include "flwr/proto/fleet.grpc.pb.h"
#include "flwr/proto/message.pb.h"
#include "message_handler.h"
#include <grpcpp/grpcpp.h>

class gRPCRereCommunicator : public Communicator {
public:
  gRPCRereCommunicator(std::string server_address, int grpc_max_message_length);

  bool send_register_node(flwr::proto::RegisterNodeFleetRequest request,
                          flwr::proto::RegisterNodeFleetResponse *response);

  bool send_activate_node(flwr::proto::ActivateNodeRequest request,
                          flwr::proto::ActivateNodeResponse *response);

  bool send_deactivate_node(flwr::proto::DeactivateNodeRequest request,
                            flwr::proto::DeactivateNodeResponse *response);

  bool send_unregister_node(flwr::proto::UnregisterNodeFleetRequest request,
                            flwr::proto::UnregisterNodeFleetResponse *response);

  bool send_pull_messages(flwr::proto::PullMessagesRequest request,
                          flwr::proto::PullMessagesResponse *response);

  bool send_push_messages(flwr::proto::PushMessagesRequest request,
                          flwr::proto::PushMessagesResponse *response);

private:
  std::unique_ptr<flwr::proto::Fleet::Stub> stub;
};

#endif
