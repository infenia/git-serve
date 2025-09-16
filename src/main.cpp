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


#include "git_serve/git_serve.hpp"
#include "git_serve/master_data.hpp"
#include "utils/logger.hpp"


int main(int argc, const char **argv) {
  logger::init();
  logger::get()->info("Git serve data fetch starting");

  Configuration conf;
  conf.populate_config();

  GitServe git_serve(conf);
  git_serve.run();

  logger::get()->info("Git serve data fetch completed");
  return 0;
}