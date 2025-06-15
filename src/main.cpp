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

#include <cstdlib>

#include "easymarry/easymarry.hpp"
#include "easymarry/master_data.hpp"
#include "rest/connection.hpp"
#include "rest/io_handler.hpp"
#include "rest/server_context.hpp"
#include "utils/logger.hpp"

int main(int argc, const char **argv) {
  Configuration conf;
  conf.populate_config();

  logger::init();
  logger::get()->info("Env loaded");
  MasterDataBuffer data_buffer;

  EasyMarry em(conf, &data_buffer);
  em.start_timer_job();

  IOHandler io_handler;
  Router router(&em, &io_handler);
  ServerContext cxt;

  cxt.port       = 9000;
  cxt.router     = &router;
  cxt.io_handler = &io_handler;

  Connection connection(&cxt);
  connection.listen();

  return 0;
}