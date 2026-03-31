/*************************************************************************************************
 *
 * @file message_handler.h
 *
 * @brief Handle server messages by calling appropriate client methods
 *
 * @author Lekang Jiang
 *
 * @version 1.0
 *
 * @date 04/09/2021
 *
 *************************************************************************************************/

#pragma once
#include "client.h"
#include "flwr/proto/message.pb.h"
#include "flwr/proto/recorddict.pb.h"
#include "serde.h"

std::tuple<flwr::proto::ClientMessage, int, bool>
handle(flwr_local::Client *client, flwr::proto::ServerMessage server_msg);

std::tuple<flwr::proto::Message, int, bool>
handle_message(flwr_local::Client *client, const flwr::proto::Message &message);
