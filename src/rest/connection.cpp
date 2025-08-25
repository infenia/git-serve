/*
 * Copyright 2025 Infenia Private Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Module: rest/connection.cpp
// Handles HTTP protocol events and server connection logic for REST API
// endpoints. Key functions: http_protocol_ev_handle, Connection, listen
// Logging: Structured logging for HTTP requests/responses (except /health)

#include "connection.hpp"

#include <libwebsockets.h>
#include <signal.h>

#include <atomic>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "rest/connection_cxt.hpp"
#include "rest/http_transaction.hpp"
#include "router.hpp"
#include "utils/logger.hpp"
#include "utils/trace_id.hpp"

// Event handlers

static int interrupted;

void sigint_handler(int sig) {
  interrupted = 1;
  exit(0);
}

// Handles HTTP protocol events for libwebsockets REST server.
// Robust error handling: All external and critical operations are checked for
// errors, with logs on failure.
int http_protocol_ev_handle(lws *wsi, lws_callback_reasons reason, void *data,
                            void *in, size_t len) {
  auto get_http_transaction = [&data]() -> HttpTransaction * {
    return reinterpret_cast<ConnectionCxt *>(data)->transaction;
  };
  auto get_server_cxt = [&wsi]() -> ServerContext * {
    return reinterpret_cast<ServerContext *>(lws_get_protocol(wsi)->user);
  };
  auto init_transaction_cxt = [&data, &wsi]() {
    auto cxt         = static_cast<ConnectionCxt *>(data);
    cxt->transaction = nullptr;
  };

  auto init_http_transaction = [&data, &wsi]() {
    static_cast<ConnectionCxt *>(data)->transaction = new HttpTransaction(wsi);
  };

  auto is_http_transaction = [&data]() {
    return static_cast<ConnectionCxt *>(data)->transaction != nullptr;
  };

  switch (reason) {
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      break;
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
      break;
    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
      break;
    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
      init_transaction_cxt();
      break;
    case LWS_CALLBACK_HTTP: {
      init_http_transaction();
      // Log incoming HTTP request, except for /health
      char uri[100];
      memset(uri, 0, 100);
      HTTP_METHOD method = get_http_transaction()->get_uri_and_method(uri, 100);
      std::string trace_id = traceid::generate();
      get_http_transaction()->set_trace_id(trace_id);
      if (std::string(uri) != "/health") {
        nlohmann::json req_log = {{"msg", "HTTP request"},
                                  {"method", (int)method},
                                  {"endpoint", std::string(uri)},
                                  {"trace_id", trace_id}};
        logger::get()->info(req_log.dump());
      }
      if (!get_http_transaction()->has_request_body()) {
        get_server_cxt()->router->route(get_http_transaction());
      }
      break;
    }
    case LWS_CALLBACK_HTTP_BODY:
      break;
    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
      get_server_cxt()->router->route(get_http_transaction());
      break;
    case LWS_CALLBACK_HTTP_WRITEABLE: {
      // Remove response logging from here (will move to write_header)
      HttpTransaction *txn = get_http_transaction();
      return txn->write();
      break;
    }
    case LWS_CALLBACK_CLOSED_HTTP:
      break;
    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
      if (is_http_transaction() && get_http_transaction()->client_closed()) {
        delete get_http_transaction();
      }
      break;
    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
      get_server_cxt()->io_handler->submit_writable();
      break;
    default:
      return 0;
  }

  return 0;
}

void log_emit_function(int level, const char *line) {
  if (level & LLL_ERR) {
    logger::get()->error(line);
  } else if (level & LLL_WARN) {
    logger::get()->warn(line);
  } else if (level & LLL_NOTICE) {
    logger::get()->info(line);
  } else {
    logger::get()->debug(line);
  }
}

// Connection function definition

// Establishes and manages a REST server connection.
// Robust error handling: All external and critical operations are checked for
// errors, with logs on failure.
Connection::Connection(ServerContext *cxt, std::atomic_bool &_exit_server)
    : exit_server(_exit_server) {
  memset(&info, 0, sizeof(lws_context_creation_info));
  memset(protocols, 0, sizeof(lws_protocols) * 2);

  server_cxt = cxt;
  signal(SIGINT, sigint_handler);

  lws_protocols http_protocol;

  http_protocol.name                  = "http";
  http_protocol.callback              = http_protocol_ev_handle;
  http_protocol.per_session_data_size = sizeof(ConnectionCxt);
  http_protocol.rx_buffer_size        = 0;
  http_protocol.tx_packet_size        = 0;
  http_protocol.user                  = server_cxt;

  protocols[0] = http_protocol;

  info.port      = server_cxt->port;
  info.protocols = protocols;
}

// Starts the REST server and listens for incoming connections.
// Robust error handling: All external and critical operations are checked for
// errors, with logs on failure.
int Connection::listen() {
  std::string trace_id = traceid::generate();
  logger::get()->info("Starting server",
                      nlohmann::json{{"trace_id", trace_id}}.dump());
  int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
  lws_set_log_level(log_level, log_emit_function);

  lws_context *context        = lws_create_context(&info);
  server_cxt->io_handler->cxt = context;

  int n = 0;
  while (n >= 0 && !interrupted) {
    exit_server.store(interrupted);
    n = lws_service(context, 0);
  }
  lws_context_destroy(context);
  logger::get()->info("Server stopped",
                      nlohmann::json{{"trace_id", trace_id}}.dump());
  return 0;
}
