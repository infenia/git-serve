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

#include "http_transaction.hpp"

#include <libwebsockets.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "utils/logger.hpp"

// HttpTransaction function definition

HttpTransaction::HttpTransaction(lws *wsi) {
  this->wsi              = wsi;
  this->data             = nullptr;
  this->data_len         = 0;
  this->pending_data_len = 0;
  this->state            = STATE::INITIALIZED;
}

void HttpTransaction::submit_write_callback() {
  lws_callback_on_writable(this->wsi);
}

int HttpTransaction::write_header() {
  memset(write_buffer, 0, sizeof(write_buffer));

  uint8_t *start = &write_buffer[LWS_PRE];
  uint8_t *p     = start;
  uint8_t *end   = &write_buffer[sizeof(write_buffer) - 1];

  if (lws_add_http_header_status(wsi, status, &p, end)) {
    return 1;
  }

  if (lws_add_http_header_content_length(wsi, data_len, &p, end)) {
    return 1;
  }
  if (this->data_len != 0) {
    if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                     (const unsigned char *)ctv, 16, &p, end)) {
      return 1;
    }
    if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_ENCODING,
                                     (const unsigned char *)cencoding, 4, &p,
                                     end)) {
      return 1;
    }
  }

  if (lws_finalize_write_http_header(wsi, start, &p, end)) {
    return 1;
  }
  // Log outgoing HTTP response ONCE per response, except for /health
  char uri[100];
  memset(uri, 0, 100);
  HTTP_METHOD method = get_uri_and_method(uri, 100);
  int status_code    = (int)get_status();
  // Only log if endpoint is not /health
  if (std::string(uri) != "/health") {
    nlohmann::json resp_log = {{"msg", "HTTP response"},
                               {"status_code", status_code},
                               {"endpoint", std::string(uri)},
                               {"trace_id", get_trace_id()}};
    logger::get()->info(resp_log.dump());
  }
  state = STATE::HEADER_WRITTEN;
  if (this->data_len != 0) {
    lws_callback_on_writable(wsi);
  } else {
    if (lws_http_transaction_completed(wsi)) {
      return 1;
    }
  }
  return 0;
}

int HttpTransaction::write_body() {
  if (pending_data_len != 0) {
    int write_len = (pending_data_len >= 4048) ? 4048 : pending_data_len;
    memset(write_buffer, 0, sizeof(write_buffer));

    memcpy(&write_buffer[LWS_PRE], &data[data_len - pending_data_len],
           write_len);
    if (lws_write(wsi, &write_buffer[LWS_PRE], write_len, LWS_WRITE_HTTP) <
        write_len) {
      return 1;
    }
    pending_data_len -= write_len;
    if (pending_data_len == 0) {
      state = STATE::BODY_WRITTEN;
      if (lws_http_transaction_completed(wsi)) {
        return 1;
      }
    } else {
      lws_callback_on_writable(wsi);
    }
  } else {
    state = STATE::BODY_WRITTEN;
    if (lws_http_transaction_completed(wsi)) return 1;
  }
  return 0;
}

int HttpTransaction::write() {
  switch (state) {
    case STATE::WRITE_SUBMITTED:
      return write_header();
    case STATE::HEADER_WRITTEN:
      return write_body();
    default:
      return -1;
  }

  return 0;
}

HTTP_METHOD HttpTransaction::get_uri_and_method(char *url, int len) {
  char *uri_buf;
  int uri_len;
  int http_method = lws_http_get_uri_and_method(wsi, &uri_buf, &uri_len);

  HTTP_METHOD method;
  switch (http_method) {
    case LWSHUMETH_GET:
      method = HTTP_METHOD::GET;
      break;
    case LWSHUMETH_HEAD:
      method = HTTP_METHOD::HEAD;
      break;
    case LWSHUMETH_POST:
      method = HTTP_METHOD::POST;
      break;
    case LWSHUMETH_PUT:
      method = HTTP_METHOD::PUT;
      break;
    case LWSHUMETH_DELETE:
      method = HTTP_METHOD::DELETE;
      break;
    case LWSHUMETH_CONNECT:
      method = HTTP_METHOD::CONNECT;
      break;
    case LWSHUMETH_OPTIONS:
      method = HTTP_METHOD::OPTIONS;
      break;
    case LWSHUMETH_PATCH:
      method = HTTP_METHOD::PATCH;
      break;
    default:
      return HTTP_METHOD::UNKNOWN;
  }

  if (uri_len >= len) {
    return HTTP_METHOD::UNKNOWN;
  }

  memcpy(url, uri_buf, uri_len);
  url[uri_len] = '\0';

  return method;
}

void HttpTransaction::set_response(http_status status, const char *data,
                                   int len) {
  this->data             = data;
  this->data_len         = len;
  this->pending_data_len = len;
  this->state            = STATE::WRITE_SUBMITTED;
  this->status           = status;
}

bool HttpTransaction::has_request_body() {
  char value[128];
  memset(value, 0, 128);
  int len = lws_hdr_copy(wsi, value, 128, WSI_TOKEN_HTTP_CONTENT_LENGTH);
  return len != 0;
}

bool HttpTransaction::client_closed() { return closed.exchange(true); }
