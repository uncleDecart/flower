#include "message_handler.h"
#include "flwr/proto/message.pb.h"
#include "flwr/proto/recorddict.pb.h"
#include <variant>

std::tuple<flwr_local::RecordSet, int>
_reconnect(flwr::proto::RecordDict proto_recorddict) {

  // Determine the reason for sending Disconnect message
  flwr::proto::Reason reason = flwr::proto::Reason::ACK;
  int sleep_duration = 0;

  // Build Disconnect message
  return std::make_tuple(
      flwr_local::RecordSet({}, {}, {{"config", {{"reason", reason}}}}),
      sleep_duration);
}

flwr_local::RecordSet _get_parameters(flwr_local::Client *client) {
  return recordset_from_get_parameters_res(client->get_parameters());
}

flwr_local::RecordSet _fit(flwr_local::Client *client,
                           flwr::proto::RecordDict proto_recorddict) {
  flwr_local::RecordSet recordset = recorddict_from_proto(proto_recorddict);
  flwr_local::FitIns fit_ins = recordset_to_fit_ins(recordset, true);
  // Perform fit
  flwr_local::FitRes fit_res = client->fit(fit_ins);

  flwr_local::RecordSet out_recordset = recordset_from_fit_res(fit_res);
  return out_recordset;
}

flwr_local::RecordSet _evaluate(flwr_local::Client *client,
                                flwr::proto::RecordDict proto_recorddict) {
  flwr_local::RecordSet recordset = recorddict_from_proto(proto_recorddict);
  flwr_local::EvaluateIns evaluate_ins =
      recordset_to_evaluate_ins(recordset, true);
  // Perform evaluation
  flwr_local::EvaluateRes evaluate_res = client->evaluate(evaluate_ins);

  flwr_local::RecordSet out_recordset =
      recordset_from_evaluate_res(evaluate_res);
  return out_recordset;
}

std::tuple<flwr_local::RecordSet, int, bool> handle(flwr_local::Client *client,
                                                    flwr::proto::Message message) {
  std::string message_type = message.metadata().message_type();
  
  if (message_type == "reconnect") {
    std::tuple<flwr_local::RecordSet, int> rec = _reconnect(message.content());
    return std::make_tuple(std::get<0>(rec), std::get<1>(rec), false);
  }
  if (message_type == "get_parameters") {
    return std::make_tuple(_get_parameters(client), 0, true);
  }
  if (message_type == "train") {
    return std::make_tuple(_fit(client, message.content()), 0, true);
  }
  if (message_type == "evaluate") {
    return std::make_tuple(_evaluate(client, message.content()), 0, true);
  }
  throw "Unknown server message";
}

std::tuple<flwr::proto::Message, int, bool>
handle_message(flwr_local::Client *client, const flwr::proto::Message &received_message) {
  
  std::tuple<flwr_local::RecordSet, int, bool> legacy_res =
      handle(client, received_message);
  auto conf_records =
      recorddict_from_proto(recorddict_to_proto(std::get<0>(legacy_res)))
          .getConfigsRecords();

  flwr::proto::Message response_message;
  
  // Copy metadata from received message and update for response
  auto metadata = response_message.mutable_metadata();
  metadata->set_run_id(received_message.metadata().run_id());
  metadata->set_message_id("");  // Will be set by server
  metadata->set_src_node_id(received_message.metadata().dst_node_id());
  metadata->set_dst_node_id(received_message.metadata().src_node_id());
  metadata->set_reply_to_message_id(received_message.metadata().message_id());
  metadata->set_group_id(received_message.metadata().group_id());
  metadata->set_ttl(3600);
  metadata->set_message_type(received_message.metadata().message_type());
  metadata->set_created_at(std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count());
  
  // Set content with the result recorddict
  *response_message.mutable_content() = recorddict_to_proto(std::get<0>(legacy_res));

  std::tuple<flwr::proto::Message, int, bool> tuple = std::make_tuple(
      response_message, std::get<1>(legacy_res), std::get<2>(legacy_res));

  return tuple;
}
