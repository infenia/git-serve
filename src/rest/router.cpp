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

#include "router.hpp"

#include <cstring>

#include "easymarry/easymarry.hpp"
#include "rest/http_transaction.hpp"

int Router::route(HttpTransaction* transaction) {
  char uri[100];
  memset(uri, 0, 100);

  switch (transaction->get_uri_and_method(uri, 100)) {
    case HTTP_METHOD::GET:
      return em->get_item(uri, transaction, io_handler);
      break;
    case HTTP_METHOD::HEAD:
      break;
    case HTTP_METHOD::POST:
      break;
    default:
      return -1;
  }
  return 0;
}
