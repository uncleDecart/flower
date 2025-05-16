#include "message_handler.h"
#include <variant>

std::tuple<flwr_local::RecordDict, int>
_reconnect(flwr::proto::RecordDict proto_recordset) {

  // Determine the reason for sending Disconnect message
  flwr::proto::Reason reason = flwr::proto::Reason::ACK;
  int sleep_duration = 0;

  // Build Disconnect message
  return std::make_tuple(
      flwr_local::RecordDict({}, {}, {{"config", {{"reason", reason}}}}),
      sleep_duration);
}

flwr_local::RecordDict _get_parameters(flwr_local::Client *client) {
  return recordset_from_get_parameters_res(client->get_parameters());
}

flwr_local::RecordDict _fit(flwr_local::Client *client,
                            flwr::proto::RecordDict proto_recordset) {
  flwr_local::RecordDict recordset = recordset_from_proto(proto_recordset);
  flwr_local::FitIns fit_ins = recordset_to_fit_ins(recordset, true);
  // Perform fit
  flwr_local::FitRes fit_res = client->fit(fit_ins);

  flwr_local::RecordDict out_recordset = recordset_from_fit_res(fit_res);
  return out_recordset;
}

flwr_local::RecordDict _evaluate(flwr_local::Client *client,
                                 flwr::proto::RecordDict proto_recordset) {
  flwr_local::RecordDict recordset = recordset_from_proto(proto_recordset);
  flwr_local::EvaluateIns evaluate_ins =
      recordset_to_evaluate_ins(recordset, true);
  // Perform evaluation
  flwr_local::EvaluateRes evaluate_res = client->evaluate(evaluate_ins);

  flwr_local::RecordDict out_recordset =
      recordset_from_evaluate_res(evaluate_res);
  return out_recordset;
}

std::tuple<flwr_local::RecordDict, int, bool>
handle(flwr_local::Client *client, flwr::proto::Message task) {
  if (task.task_type() == "reconnect") {
    std::tuple<flwr_local::RecordDict, int> rec = _reconnect(task.recordset());
    return std::make_tuple(std::get<0>(rec), std::get<1>(rec), false);
  }
  if (task.task_type() == "get_parameters") {
    return std::make_tuple(_get_parameters(client), 0, true);
  }
  if (task.task_type() == "train") {
    return std::make_tuple(_fit(client, task.recordset()), 0, true);
  }
  if (task.task_type() == "evaluate") {
    return std::make_tuple(_evaluate(client, task.recordset()), 0, true);
  }
  throw "Unkown server message";
}

std::tuple<flwr::proto::Message, int, bool>
handle_task(flwr_local::Client *client, const flwr::proto::Message &task_ins) {
  flwr::proto::Message received_task = task_ins.task();

  std::tuple<flwr_local::RecordDict, int, bool> legacy_res =
      handle(client, received_task);
  auto conf_records =
      recordset_from_proto(recordset_to_proto(std::get<0>(legacy_res)))
          .getConfigsRecords();

  flwr::proto::Message task_res;

  task_res.set_task_id("");
  task_res.set_group_id(task_ins.group_id());
  task_res.set_run_id(task_ins.run_id());

  std::unique_ptr<flwr::proto::Message> task =
      std::make_unique<flwr::proto::Message>();

  std::unique_ptr<flwr::proto::RecordDict> proto_recordset_ptr =
      std::make_unique<flwr::proto::RecordDict>(
          recordset_to_proto(std::get<0>(legacy_res)));

  task->set_allocated_recordset(proto_recordset_ptr.release());
  task->set_task_type(received_task.task_type());
  task->set_ttl(3600);
  task->set_created_at(std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
  task->set_allocated_consumer(
      std::make_unique<flwr::proto::Node>(received_task.producer()).release());
  task->set_allocated_producer(
      std::make_unique<flwr::proto::Node>(received_task.consumer()).release());

  task_res.set_allocated_task(task.release());

  std::tuple<flwr::proto::Message, int, bool> tuple = std::make_tuple(
      task_res, std::get<1>(legacy_res), std::get<2>(legacy_res));

  return tuple;
}
