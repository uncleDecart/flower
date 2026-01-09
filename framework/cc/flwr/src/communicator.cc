#include "communicator.h"
#include <map>
#include <mutex>
#include <iostream>

const std::string KEY_NODE = "node";
const std::string KEY_MESSAGE = "current_message";

std::map<std::string, std::optional<flwr::proto::Node>> node_store;
std::map<std::string, std::optional<flwr::proto::Message>> state;

std::mutex node_store_mutex;
std::mutex state_mutex;

std::optional<flwr::proto::Node> get_node_from_store() {
  std::lock_guard<std::mutex> lock(node_store_mutex);
  auto node = node_store.find(KEY_NODE);
  if (node == node_store.end() || !node->second.has_value()) {
    std::cerr << "Node instance missing" << std::endl;
    return std::nullopt;
  }
  return node->second;
}

bool validate_message(const flwr::proto::Message &message,
                      const bool discard_reconnect) {
  return message.has_metadata() && message.has_content();
}

flwr::proto::Message
configure_message(const flwr::proto::Message &message,
                  const flwr::proto::Message &ref_message,
                  const flwr::proto::Node &producer) {
  flwr::proto::Message result_message;
  
  // Copy content from input message
  *result_message.mutable_content() = message.content();
  
  // Configure metadata
  auto metadata = result_message.mutable_metadata();
  metadata->set_run_id(ref_message.metadata().run_id());
  metadata->set_message_id("");
  metadata->set_src_node_id(producer.node_id());
  metadata->set_dst_node_id(ref_message.metadata().src_node_id());
  metadata->set_reply_to_message_id(ref_message.metadata().message_id());
  metadata->set_group_id(ref_message.metadata().group_id());
  metadata->set_ttl(ref_message.metadata().ttl());
  metadata->set_message_type(message.metadata().message_type());
  
  return result_message;
}

void delete_node_from_store() {
  std::lock_guard<std::mutex> lock(node_store_mutex);
  auto node = node_store.find(KEY_NODE);
  if (node == node_store.end() || !node->second.has_value()) {
    node_store.erase(node);
  }
}

std::optional<flwr::proto::Message> get_current_message() {
  std::lock_guard<std::mutex> state_lock(state_mutex);
  auto current_message = state.find(KEY_MESSAGE);
  if (current_message == state.end() ||
      !current_message->second.has_value()) {
    std::cerr << "No current Message" << std::endl;
    return std::nullopt;
  }
  return current_message->second;
}

void create_node(Communicator *communicator) {
  flwr::proto::CreateNodeRequest create_node_request;
  flwr::proto::CreateNodeResponse create_node_response;

  create_node_request.set_ping_interval(300.0);

  communicator->send_create_node(create_node_request, &create_node_response);

  // Validate the response
  if (!create_node_response.has_node()) {
    std::cerr << "Received response does not contain a node." << std::endl;
    return;
  }

  {
    std::lock_guard<std::mutex> lock(node_store_mutex);
    node_store[KEY_NODE] = create_node_response.node();
  }
}

void delete_node(Communicator *communicator) {
  auto node = get_node_from_store();
  if (!node) {
    return;
  }
  flwr::proto::DeleteNodeRequest delete_node_request;
  flwr::proto::DeleteNodeResponse delete_node_response;

  *delete_node_request.mutable_node() = *node;

  if (!communicator->send_delete_node(delete_node_request,
                                      &delete_node_response)) {
    return;
  }

  delete_node_from_store();
}

std::optional<flwr::proto::Message> receive(Communicator *communicator) {
  auto node = get_node_from_store();
  if (!node) {
    return std::nullopt;
  }
  flwr::proto::PullMessagesResponse response;
  flwr::proto::PullMessagesRequest request;

  *request.mutable_node() = *node;

  bool success = communicator->send_pull_messages(request, &response);

  if (!success) {
    return std::nullopt;
  }

  if (response.messages_list_size() > 0) {
    flwr::proto::Message message = response.messages_list().at(0);
    if (validate_message(message, true)) {
      std::lock_guard<std::mutex> state_lock(state_mutex);
      state[KEY_MESSAGE] = message;
      return message;
    }
  }
  std::cerr << "Messages list is empty." << std::endl;
  return std::nullopt;
}

void send(Communicator *communicator, flwr::proto::Message message) {
  auto node = get_node_from_store();
  if (!node) {
    return;
  }

  auto ref_message = get_current_message();
  if (!ref_message) {
    return;
  }

  if (!validate_message(message, false)) {
    std::cerr << "Message is invalid" << std::endl;
    std::lock_guard<std::mutex> state_lock(state_mutex);
    state[KEY_MESSAGE].reset();
    return;
  }

  flwr::proto::Message new_message =
      configure_message(message, *ref_message, *node);

  flwr::proto::PushMessagesRequest request;
  *request.mutable_node() = *node;
  *request.add_messages_list() = new_message;
  flwr::proto::PushMessagesResponse response;

  communicator->send_push_messages(request, &response);

  {
    std::lock_guard<std::mutex> state_lock(state_mutex);
    state[KEY_MESSAGE].reset();
  }
}
