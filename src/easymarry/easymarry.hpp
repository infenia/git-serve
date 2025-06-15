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

 public:
  MasterDataBuffer* data_buffer = nullptr;

  EasyMarry(Configuration& conf, MasterDataBuffer* data_buffer) {
    this->conf        = conf;
    this->data_buffer = data_buffer;
  }

  void start_timer_job() {
    t1 = std::thread([this]() {
      while (true) {
        update_cache();
        sleep(60 * 60);
      }
    });
  }

  void update_cache() {
    std::string token  = conf.token;
    std::string repo   = conf.repo;
    std::string owner  = conf.owner;
    std::string branch = conf.branch;

    auto health = get_files_from_github(token, owner, repo, branch, "health/status.json");

    auto age = get_files_from_github(token, owner, repo, branch, "data/age.json");

    auto caste =
        get_files_from_github(token, owner, repo, branch, "data/caste.json");

    auto country =
        get_files_from_github(token, owner, repo, branch, "data/country.json");

    auto designation =
        get_files_from_github(token, owner, repo, branch, "data/designation.json");

    auto im_signing_up_for = get_files_from_github(
        token, owner, repo, branch, "data/im-signing-up-for.json");

    auto indian_state_district = get_files_from_github(
        token, owner, repo, branch, "data/indian-state-district.json");

    auto indian_state =
        get_files_from_github(token, owner, repo, branch, "data/indian-state.json");

    auto location =
        get_files_from_github(token, owner, repo, branch, "data/location.json");

    auto qualification =
        get_files_from_github(token, owner, repo, branch, "data/qualification.json");

    auto religion =
        get_files_from_github(token, owner, repo, branch, "data/religion.json");

    auto sub_caste =
        get_files_from_github(token, owner, repo, branch, "data/sub-caste.json");

    if (health) data_buffer->update("/health", health);

    if (age) data_buffer->update("/age", age);

    if (caste) data_buffer->update("/caste", caste);

    if (country) data_buffer->update("/country", country);

    if (designation) data_buffer->update("/designation", designation);

    if (im_signing_up_for)
      data_buffer->update("/im-signing-up-for", im_signing_up_for);

    if (indian_state_district)
      data_buffer->update("/indian-state-district", indian_state_district);

    if (indian_state) data_buffer->update("/indian-state", indian_state);

    if (location) data_buffer->update("/location", location);

    if (qualification) data_buffer->update("/qualification", qualification);

    if (religion) data_buffer->update("/religion", religion);

    if (sub_caste) data_buffer->update("/sub-caste", sub_caste);

    auto buff = get_files_from_github(token, owner, repo, branch, "data/post.csv");

    if (buff) {
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