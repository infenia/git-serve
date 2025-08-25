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
#include "rest/http_transaction.hpp"
#include "rest/io_handler.hpp"

int serve_item(const std::string loc, HttpTransaction* transaction,
               IOHandler* io_handler, MasterDataBuffer* data_buffer);

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
        for (int i = 0; i < 60 * 5 && !interrupted.load(); ++i) {
          sleep(1);
        }
      }
    });
  }

  void update_cache() {
    std::string token  = conf.token;
    std::string repo   = conf.repo;
    std::string owner  = conf.owner;
    std::string branch = conf.branch;

    if (auto health = get_files_from_github(token, owner, repo, branch,
                                            "health/status.json");
        health) {
      data_buffer->update("/health", health);
      delete health;
    }

    if (auto age = get_files_from_github(token, owner, repo, branch,
                                         "resource/age.json");
        age) {
      data_buffer->update("/age", age);
      delete age;
    }

    if (auto caste = get_files_from_github(token, owner, repo, branch,
                                           "resource/caste.json");
        caste) {
      data_buffer->update("/caste", caste);
      delete caste;
    }

    if (auto country = get_files_from_github(token, owner, repo, branch,
                                             "resource/country.json");
        country) {
      data_buffer->update("/country", country);
      delete country;
    }

    if (auto designation = get_files_from_github(token, owner, repo, branch,
                                                 "resource/designation.json");
        designation) {
      data_buffer->update("/designation", designation);
      delete designation;
    }

    if (auto im_signing_up_for = get_files_from_github(
            token, owner, repo, branch, "resource/im-signing-up-for.json");
        im_signing_up_for) {
      data_buffer->update("/im-signing-up-for", im_signing_up_for);
      delete im_signing_up_for;
    }

    if (auto indian_state_district = get_files_from_github(
            token, owner, repo, branch, "resource/indian-state-district.json");
        indian_state_district) {
      data_buffer->update("/indian-state-district", indian_state_district);
      delete indian_state_district;
    }

    if (auto indian_state = get_files_from_github(token, owner, repo, branch,
                                                  "resource/indian-state.json");
        indian_state) {
      data_buffer->update("/indian-state", indian_state);
      delete indian_state;
    }

    if (auto location = get_files_from_github(token, owner, repo, branch,
                                              "resource/location.json");
        location) {
      data_buffer->update("/location", location);
      delete location;
    }

    if (auto qualification = get_files_from_github(
            token, owner, repo, branch, "resource/qualification.json");
        qualification) {
      data_buffer->update("/qualification", qualification);
      delete qualification;
    }

    if (auto religion = get_files_from_github(token, owner, repo, branch,
                                              "resource/religion.json");
        religion) {
      data_buffer->update("/religion", religion);
      delete religion;
    }

    if (auto sub_caste = get_files_from_github(token, owner, repo, branch,
                                               "resource/sub-caste.json");
        sub_caste) {
      data_buffer->update("/sub-caste", sub_caste);
      delete sub_caste;
    }

    if (auto buff = get_files_from_github(token, owner, repo, branch,
                                          "resource/post.csv");
        buff) {
      update_postalcodes_cache(buff, data_buffer);
      delete buff;
    }
  }

  int get_item(std::string url, HttpTransaction* transaction,
               IOHandler* io_handler) {
    serve_item(url, transaction, io_handler, data_buffer);
    return 0;
  }
};