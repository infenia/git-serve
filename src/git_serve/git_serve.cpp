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

// Module: easymarry/easymarry.cpp
// Implements business logic for application-specific workflows and external API
// calls (e.g., GitHub). Error Handling: Uses retry logic and logs failures for
// all external calls.

#include "git_serve.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstring>
#include <forward_list>
#include <fstream>
#include <list>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <unordered_map>

#include "git_serve/master_data.hpp"
#include "utils/logger.hpp"
#include "utils/trace_id.hpp"

off_t get_file_size(int fd) {
  struct stat st;
  if (fstat(fd, &st) < 0) {
    logger::get()->error("fstat failed");
    return -1;
  }
  if (S_ISBLK(st.st_mode)) {
    unsigned long long bytes;
    if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
      logger::get()->error("ioctl failed");
      return -1;
    }
    return bytes;
  } else if (S_ISREG(st.st_mode))
    return st.st_size;
  return -1;
}

std::string postalcode_to_json(PostalCode& postalCode) {
  auto gen_keyvalue = [&](const std::string& k, const std::string& v) {
    std::string json = "\"" + k + "\":\"" + v + "\"";
    return json;
  };
  std::string json = "{";
  json += gen_keyvalue("name", postalCode.name) + ",";
  json += gen_keyvalue("pincode", postalCode.pinCode) + ",";
  json += gen_keyvalue("division", postalCode.division) + ",";
  json += gen_keyvalue("region", postalCode.region) + ",";
  json += gen_keyvalue("circle", postalCode.circle) + ",";
  json += gen_keyvalue("taluk", postalCode.taluk) + ",";
  json += gen_keyvalue("district", postalCode.district) + ",";
  json += gen_keyvalue("state", postalCode.state) + ",";
  json += gen_keyvalue("longitude", postalCode.longitude) + ",";
  json += gen_keyvalue("latitude", postalCode.latitude) + "}";
  return json;
}

std::string postalcodes_to_json(std::list<PostalCode>& postalCodes) {
  std::string json       = "[";
  std::string item_split = "";
  for (auto& postalCode : postalCodes) {
    json += item_split;
    json += postalcode_to_json(postalCode);
    item_split = ",";
  }
  json += "]\0";
  return json;
}

void update_postalcodes_cache(const char* buff, GitServe* git_serve) {
  std::string csv(buff);
  std::stringstream csv_stream(csv);
  std::string csv_line;

  std::unordered_map<std::string, std::list<PostalCode>> postalCodes_map;
  while (std::getline(csv_stream, csv_line, '\n')) {
    std::stringstream csv_line_stream(csv_line);

    std::string item;
    PostalCode postalCode;

    auto get_next = [&]() {
      std::string item;
      std::getline(csv_line_stream, item, ',');
      return item;
    };
    postalCode.name      = get_next();
    postalCode.pinCode   = get_next();
    postalCode.division  = get_next();
    postalCode.region    = get_next();
    postalCode.circle    = get_next();
    postalCode.taluk     = get_next();
    postalCode.district  = get_next();
    postalCode.state     = get_next();
    postalCode.longitude = get_next();
    postalCode.latitude  = get_next();

    if (postalCodes_map.contains(postalCode.pinCode)) {
      postalCodes_map[postalCode.pinCode].push_back(postalCode);
    } else {
      postalCodes_map[postalCode.pinCode] = std::list<PostalCode>({postalCode});
    }
  }

  for (auto& grouped_postalcodes : postalCodes_map) {
    auto json_array = postalcodes_to_json(grouped_postalcodes.second);
    git_serve->update_data("/postalcode/" + grouped_postalcodes.first,
                           json_array);
  }
}

struct ResponseData {
  char* data;
  size_t size;
};

// Callback function to store the received data
size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  size_t total_size        = size * nmemb;
  struct ResponseData* res = (struct ResponseData*)userdata;
  std::string trace_id     = traceid::generate();

  char* temp = (char*)realloc(res->data, res->size + total_size + 1);
  if (temp == NULL) {
    logger::get()->error("Memory allocation failed",
                         nlohmann::json{{"trace_id", trace_id}}.dump());
    return 0;
  }

  res->data = temp;
  memcpy(res->data + res->size, ptr, total_size);
  res->size += total_size;
  res->data[res->size] = '\0';  // Null-terminate the string

  return total_size;
}

char* get_files_from_github(const std::string& token, const std::string& owner,
                            const std::string& repo, const std::string& branch,
                            const std::string& path) {
  std::string url = "https://api.github.com/repos/" + owner + "/" + repo +
                    "/contents/" + path + "?ref=" + branch;
  std::string auth_header = "Authorization: Bearer " + token;
  CURL* curl;
  CURLcode res;
  ResponseData data;
  std::string trace_id = traceid::generate();
  data.data            = (char*)malloc(1);  // Allocate initial memory
  data.size            = 0;

  curl = curl_easy_init();
  if (curl) {
    curl_slist* headers = NULL;
    headers =
        curl_slist_append(headers, "Accept: application/vnd.github.raw+json");
    headers = curl_slist_append(headers, auth_header.c_str());
    headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "easymarry/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 20 * 1024 * 1024);

    int max_retries     = 3;
    int retry_delay_sec = 2;
    int attempt         = 0;
    bool success        = false;
    for (attempt = 0; attempt < max_retries; ++attempt) {
      // Log each attempt to call GitHub
      nlohmann::json call_log = {{"msg", "GitHub API call"},
                                 {"url", url},
                                 {"trace_id", trace_id},
                                 {"attempt", attempt + 1}};
      logger::get()->info(call_log.dump());
      res = curl_easy_perform(curl);
      if (res == CURLE_OK) {
        // Log GitHub call success
        nlohmann::json success_log = {{"msg", "GitHub API call success"},
                                      {"url", url},
                                      {"trace_id", trace_id},
                                      {"attempt", attempt + 1}};
        logger::get()->info(success_log.dump());
        success = true;
        break;
      } else {
        const char* err_str = curl_easy_strerror(res);
        std::string error_msg =
            std::string("curl_easy_perform() failed: ") + err_str;
        nlohmann::json log_obj = {{"msg", error_msg},
                                  {"trace_id", trace_id},
                                  {"url", url},
                                  {"attempt", attempt + 1}};
        logger::get()->error(log_obj.dump());
        sleep(retry_delay_sec);
      }
    }

    if (attempt >= max_retries) {
      std::string error_msg =
          std::string("Fetching content from github failed with retries");
      nlohmann::json log_obj = {{"msg", error_msg},
                                {"trace_id", trace_id},
                                {"url", url},
                                {"attempt", attempt + 1}};
      logger::get()->error(log_obj.dump());
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (success) {
      return data.data;
    } else {
      free(data.data);
      return nullptr;
    }
  }

  // Clean up if curl_easy_init failed
  free(data.data);
  return nullptr;
}

void GitServe::update_cache() {
  std::string token  = conf.token;
  std::string repo   = conf.repo;
  std::string owner  = conf.owner;
  std::string branch = conf.branch;

  if (auto health = get_files_from_github(token, owner, repo, branch,
                                          "health/status.json");
      health) {
    update_data("/health", std::string(health));
    free(health);
  }

  if (auto age = get_files_from_github(token, owner, repo, branch,
                                       "resource/age.json");
      age) {
    update_data("/age", std::string(age));
    free(age);
  }

  if (auto caste = get_files_from_github(token, owner, repo, branch,
                                         "resource/caste.json");
      caste) {
    update_data("/caste", std::string(caste));
    free(caste);
  }

  if (auto country = get_files_from_github(token, owner, repo, branch,
                                           "resource/country.json");
      country) {
    update_data("/country", std::string(country));
    free(country);
  }

  if (auto designation = get_files_from_github(token, owner, repo, branch,
                                               "resource/designation.json");
      designation) {
    update_data("/designation", std::string(designation));
    free(designation);
  }

  if (auto im_signing_up_for = get_files_from_github(
          token, owner, repo, branch, "resource/im-signing-up-for.json");
      im_signing_up_for) {
    update_data("/im-signing-up-for", std::string(im_signing_up_for));
    free(im_signing_up_for);
  }

  if (auto indian_state_district = get_files_from_github(
          token, owner, repo, branch, "resource/indian-state-district.json");
      indian_state_district) {
    update_data("/indian-state-district", std::string(indian_state_district));
    free(indian_state_district);
  }

  if (auto indian_state = get_files_from_github(token, owner, repo, branch,
                                                "resource/indian-state.json");
      indian_state) {
    update_data("/indian-state", std::string(indian_state));
    free(indian_state);
  }

  if (auto location = get_files_from_github(token, owner, repo, branch,
                                            "resource/location.json");
      location) {
    update_data("/location", std::string(location));
    free(location);
  }

  if (auto qualification = get_files_from_github(token, owner, repo, branch,
                                                 "resource/qualification.json");
      qualification) {
    update_data("/qualification", std::string(qualification));
    free(qualification);
  }

  if (auto religion = get_files_from_github(token, owner, repo, branch,
                                            "resource/religion.json");
      religion) {
    update_data("/religion", std::string(religion));
    free(religion);
  }

  if (auto sub_caste = get_files_from_github(token, owner, repo, branch,
                                             "resource/sub-caste.json");
      sub_caste) {
    update_data("/sub-caste", std::string(sub_caste));
    free(sub_caste);
  }

  if (auto buff = get_files_from_github(token, owner, repo, branch,
                                        "resource/post.csv");
      buff) {
    update_postalcodes_cache(buff, this);
    free(buff);
  }
}

void GitServe::update_data(const std::string& item, const std::string& value) {
  std::string file_path = "/var/www/html" + item + ".json";

  // Check if file exists and has the same content
  std::ifstream existing_file(file_path);
  if (existing_file.is_open()) {
    std::string existing_content(
        (std::istreambuf_iterator<char>(existing_file)),
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
