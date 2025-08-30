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

#include <fstream>
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


struct MasterDataBuffer {
  void update(const std::string& item, const std::string& value) {
    std::string file_path = "/var/www/html" + item + ".json";

    // Check if file exists and has the same content
    std::ifstream existing_file(file_path);
    if (existing_file.is_open()) {
      std::string existing_content((std::istreambuf_iterator<char>(existing_file)),
                                   std::istreambuf_iterator<char>());
      existing_file.close();
      
      // If content is identical, don't write (preserves modification time)
      if (existing_content == value) {
        return;
      }
    }

    // Create directory structure if needed
    size_t last_slash = file_path.find_last_of('/');
    if (last_slash != std::string::npos) {
      std::string dir_path  = file_path.substr(0, last_slash);
      std::string mkdir_cmd = "mkdir -p " + dir_path;
      system(mkdir_cmd.c_str());
    }

    // Write data directly to file only if content changed
    std::ofstream file(file_path, std::ios::out);
    if (file.is_open()) {
      file << value;
      file.close();
    }
  }

};