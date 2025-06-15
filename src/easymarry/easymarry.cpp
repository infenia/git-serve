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
// Implements business logic for application-specific workflows and external API calls (e.g., GitHub).
// Error Handling: Uses retry logic and logs failures for all external calls.

#include "easymarry.hpp"

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
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>

#include "easymarry/master_data.hpp"
#include "rest/http_transaction.hpp"
#include "rest/io_handler.hpp"
#include "utils/logger.hpp"
#include "utils/trace_id.hpp"
#include <nlohmann/json.hpp>

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

void update_item_cache(const std::string& buff, const std::string& loc,
                       MasterDataBuffer* data_buffer) {
  char* data = new char[buff.length() + 1];
  memset(data, 0, buff.length() + 1);
  strncpy(data, buff.c_str(), buff.length());

  data_buffer->update(loc, data);
}

void update_postalcodes_cache(const char* buff, MasterDataBuffer* data_buffer) {
  std::string csv(buff);
  std::stringstream csv_stream(csv);
  std::string csv_line;

  std::forward_list<PostalCode> postalCodes;
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
    postalCodes.push_front(postalCode);
  }

  std::unordered_map<std::string, std::list<PostalCode>> postalCodes_map;

  for (auto& postalCode : postalCodes) {
    auto json_item = postalcode_to_json(postalCode);
    if (postalCodes_map.contains(postalCode.pinCode)) {
      postalCodes_map[postalCode.pinCode].push_back(postalCode);
    } else {
      postalCodes_map[postalCode.pinCode] = std::list<PostalCode>({postalCode});
    }
  }

  for (auto& grouped_postalcodes : postalCodes_map) {
    auto json_array = postalcodes_to_json(grouped_postalcodes.second);
    data_buffer->update("/postalcode/" + grouped_postalcodes.first, json_array);
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
  std::string trace_id = traceid::generate();

  char* temp = (char*)realloc(res->data, res->size + total_size + 1);
  if (temp == NULL) {
    logger::get()->error("Memory allocation failed", nlohmann::json{{"trace_id", trace_id}}.dump());
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
  std::string url =
      "https://api.github.com/repos/" + owner + "/" + repo + "/contents/" + path  + "?ref=" + branch;
  std::string auth_header = "Authorization: Bearer " + token;
  CURL* curl;
  CURLcode res;
  ResponseData data;
  std::string trace_id = traceid::generate();
  data.data = (char*)malloc(1);  // Allocate initial memory
  data.size = 0;

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

    int max_retries = 3;
    int retry_delay_sec = 2;
    int attempt = 0;
    bool success = false;
    for (attempt = 0; attempt < max_retries; ++attempt) {
      // Log each attempt to call GitHub
      nlohmann::json call_log = {
        {"msg", "GitHub API call"},
        {"url", url},
        {"trace_id", trace_id},
        {"attempt", attempt + 1}
      };
      logger::get()->info(call_log.dump());
      res = curl_easy_perform(curl);
      if (res == CURLE_OK) {
        // Log GitHub call success
        nlohmann::json success_log = {
          {"msg", "GitHub API call success"},
          {"url", url},
          {"trace_id", trace_id},
          {"attempt", attempt + 1}
        };
        logger::get()->info(success_log.dump());
        success = true;
        break;
      } else {
        const char* err_str = curl_easy_strerror(res);
        std::string error_msg = std::string("curl_easy_perform() failed: ") + err_str;
        nlohmann::json log_obj = {
          {"msg", error_msg},
          {"trace_id", trace_id},
          {"url", url},
          {"attempt", attempt + 1}
        };
        logger::get()->error(log_obj.dump());
        sleep(retry_delay_sec);
      }
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
  return nullptr;
}

int serve_item(const std::string loc, HttpTransaction* transaction,
               IOHandler* io_handler, MasterDataBuffer* data_buffer) {
  auto data = data_buffer->get_item(loc);
  if (data != nullptr) {
    transaction->set_response(http_status::HTTP_STATUS_OK, data->data,
                              data->len);
    io_handler->write_response(transaction);
    return 0;
  } else {
    transaction->set_response(http_status::HTTP_STATUS_NOT_FOUND, nullptr, 0);
    io_handler->write_response(transaction);
    return 0;
  }
}