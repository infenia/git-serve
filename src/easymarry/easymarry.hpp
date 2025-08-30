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
#include <unistd.h>

#include <atomic>
#include <string>
#include <thread>

#include "easymarry/master_data.hpp"
#include "malloc.h"

char* get_files_from_github(const std::string& token, const std::string& owner,
                            const std::string& repo, const std::string& branch,
                            const std::string& path);

void update_postalcodes_cache(const char* buff, MasterDataBuffer* data_buffer);

class EasyMarry {
  Configuration conf;
  std::thread t1;
  std::atomic_bool& interrupted;

 public:
  MasterDataBuffer* data_buffer = nullptr;

  EasyMarry(Configuration& _conf, MasterDataBuffer* _data_buffer,
            std::atomic_bool& _interrupted)
      : conf(_conf), data_buffer(_data_buffer), interrupted(_interrupted) {}

  ~EasyMarry() {
    if (t1.joinable()) {
      t1.join();
    }
  }

  void start_timer_job() {
    t1 = std::thread([this]() {
      while (!interrupted.load()) {
        update_cache();
        malloc_trim(0);
        for (int i = 0; i < 60 * 60 && !interrupted.load(); ++i) {
          sleep(1);
        }
      }
    });
  }

  void shutdown() {
    interrupted.store(true);
    if (t1.joinable()) {
      t1.join();
    }
  }

  void update_cache();
};