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
  // Step 1: Register the node
  flwr::proto::RegisterNodeFleetRequest register_request;
  flwr::proto::RegisterNodeFleetResponse register_response;
  
  // Set a dummy public key for now (in real implementation, this should be generated)
  register_request.set_public_key("dummy_public_key");
  
  if (!communicator->send_register_node(register_request, &register_response)) {
    std::cerr << "Failed to register node." << std::endl;
    return;
  }
  
  uint64_t node_id = register_response.node_id();
  
  // Step 2: Activate the node
  flwr::proto::ActivateNodeRequest activate_request;
  flwr::proto::ActivateNodeResponse activate_response;
  
  activate_request.set_public_key("dummy_public_key");
  activate_request.set_heartbeat_interval(300.0);
  
  if (!communicator->send_activate_node(activate_request, &activate_response)) {
    std::cerr << "Failed to activate node." << std::endl;
    return;
  }
  
  // Create a Node object and store it
  flwr::proto::Node node;
  node.set_node_id(node_id);
  
  {
    std::lock_guard<std::mutex> lock(node_store_mutex);
    node_store[KEY_NODE] = node;
  }
}

void delete_node(Communicator *communicator) {
  auto node = get_node_from_store();
  if (!node) {
    return;
  }
  
  uint64_t node_id = node->node_id();
  
  // Step 1: Deactivate the node
  flwr::proto::DeactivateNodeRequest deactivate_request;
  flwr::proto::DeactivateNodeResponse deactivate_response;
  
  deactivate_request.set_node_id(node_id);
  
  if (!communicator->send_deactivate_node(deactivate_request, &deactivate_response)) {
    std::cerr << "Failed to deactivate node." << std::endl;
    // Continue to unregister even if deactivate fails
  }
  
  // Step 2: Unregister the node
  flwr::proto::UnregisterNodeFleetRequest unregister_request;
  flwr::proto::UnregisterNodeFleetResponse unregister_response;
  
  unregister_request.set_node_id(node_id);
  
  if (!communicator->send_unregister_node(unregister_request, &unregister_response)) {
    std::cerr << "Failed to unregister node." << std::endl;
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
