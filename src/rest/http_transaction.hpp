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

#pragma once

#include <libwebsockets.h>

#include <atomic>
#include <string>

enum class HTTP_METHOD {
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  PATCH,
  UNKNOWN
};

class HttpTransaction {
  enum class STATE {
    INITIALIZED,
    RECEIVED,
    WRITE_SUBMITTED,
    HEADER_WRITTEN,
    BODY_WRITTEN,
    CLIENT_CLOSED,
    DONE
  };
  uint8_t write_buffer[LWS_PRE + 4048];
  char const *data;
  int data_len;
  int pending_data_len;
  http_status status;
  STATE state;
  std::atomic_bool closed;
  std::string trace_id;

  int write_body();
  int write_header();

  const char *ctv       = "application/json";
  const char *cencoding = "gzip";

 public:
  lws *wsi;
  HttpTransaction(lws *wsi);
  void submit_write_callback();
  int write();
  void set_response(http_status status, const char *data, int len);
  HTTP_METHOD get_uri_and_method(char *url, int len);
  bool has_request_body();
  bool client_closed();
  http_status get_status() const { return status; }
  void set_trace_id(const std::string& id) { trace_id = id; }
  const std::string& get_trace_id() const { return trace_id; }
};