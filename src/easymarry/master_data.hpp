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
#include <zlib.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

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
    token = std::string(std::getenv("GITHUB_TOKEN"));
    owner  = std::string(std::getenv("GITHUB_OWNER"));
    repo   = std::string(std::getenv("GITHUB_REPO"));
    branch = std::string(std::getenv("GITHUB_BRANCH"));
  }
};

struct ResponseBuffer {
  char* data;
  int len;
};

struct MasterDataBuffer {
  bool updated               = false;
  std::atomic_bool spin_lock = false;
  std::unordered_map<std::string, ResponseBuffer*> master_data;

  std::string compressGzip(const std::string& data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8,
                     Z_DEFAULT_STRATEGY) != Z_OK)
      throw std::runtime_error("deflateInit2 failed");

    zs.next_in  = (Bytef*)data.data();
    zs.avail_in = data.size();

    std::string outString;
    const size_t bufferSize = 32768;
    unsigned char tempBuffer[bufferSize];

    int ret;
    do {
      zs.next_out  = tempBuffer;
      zs.avail_out = bufferSize;

      ret = deflate(&zs, Z_FINISH);
      if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) break;

      outString.append(reinterpret_cast<char*>(tempBuffer),
                       bufferSize - zs.avail_out);
    } while (ret != Z_STREAM_END);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) throw std::runtime_error("deflate failed");

    return outString;
  }

  ResponseBuffer* get_item(const std::string& item) {
    if (master_data.contains(item)) {
      return master_data[item];
    }
    return nullptr;
  }
  void update(const std::string& item, char* raw_value) {
    auto compressed_value = compressGzip(raw_value);

    char* value = new char[compressed_value.size() + 1];
    memset(value, 0, compressed_value.size() + 1);
    std::copy(compressed_value.begin(), compressed_value.end(), value);
    free(raw_value);
    if (master_data.contains(item)) {
      free(master_data[item]);
      master_data[item]->data = value;
      master_data[item]->len  = compressed_value.length();
    } else {
      ResponseBuffer* buffer = new ResponseBuffer;
      buffer->data           = value;
      buffer->len            = compressed_value.length();
      master_data[item]      = buffer;
    }
  }

  void update(const std::string& item, const std::string& value) {
    char* data = (char*)malloc(value.length() + 1);
    memset(data, 0, value.length() + 1);
    strncpy(data, value.c_str(), value.length());
    update(item, data);
  }
};