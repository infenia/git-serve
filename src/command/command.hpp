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

#include <iostream>
#include "utils/logger.hpp"
#include "traceid.hpp"
#include <nlohmann/json.hpp>

/**
 * @brief Abstract base class for all commands.
 */
class Command {
 public:
  Command() {}
  /**
   * @brief Execute the command.
   */
  virtual void execute() = 0;

  virtual ~Command() {}
};

/**
 * @brief Command to create a schema.
 */
class CreateSchema : public Command {
 public:
  CreateSchema() {}
  /**
   * @brief Execute the CreateSchema command.
   * Logs the action with context.
   */
  virtual void execute() override {
    std::string trace_id = traceid::generate();
    logger::get()->info("Create schema", nlohmann::json{{"trace_id", trace_id}}.dump());
  }
};