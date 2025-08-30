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

#include <string>

struct PostalCode {
  std::string name;
  std::string pinCode;
  std::string division;
  std::string region;
  std::string circle;
  std::string taluk;
  std::string district;
  std::string state;
  std::string longitude;
  std::string latitude;
};

struct Configuration {
  std::string token;
  std::string owner;
  std::string repo;
  std::string branch;

  void populate_config() {
    token  = std::string(std::getenv("GITHUB_TOKEN"));
    owner  = std::string(std::getenv("GITHUB_OWNER"));
    repo   = std::string(std::getenv("GITHUB_REPO"));
    branch = std::string(std::getenv("GITHUB_BRANCH"));
  }
};


