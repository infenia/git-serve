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

#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/fmt.h>
#include <nlohmann/json.hpp>
#include <iostream>

namespace logger {
    static std::shared_ptr<spdlog::logger> logger_instance = nullptr;

    void init() {
        if (!logger_instance) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("{\"timestamp\": \"%Y-%m-%dT%H:%M:%S.%eZ\", \"level\": \"%l\", \"message\": \"%v\"}");
            logger_instance = std::make_shared<spdlog::logger>("json_logger", console_sink);
            // Set only your logger to info
            logger_instance->set_level(spdlog::level::info);
            // Set the global default logger (used by libraries) to warn
            spdlog::set_level(spdlog::level::warn);
            spdlog::set_default_logger(logger_instance);
        }
    }

    std::shared_ptr<spdlog::logger> get() {
        if (!logger_instance) init();
        return logger_instance;
    }

    void log_to_gcp(const std::string& message, spdlog::level::level_enum level) {
        // If running on GCP, logs to stdout/stderr will be ingested automatically.
        // For direct API integration, use the Google Cloud Logging C++ client.
        // This is a stub for future expansion.
        nlohmann::json log_entry = {
            {"message", message},
            {"level", spdlog::level::to_string_view(level)}
        };
        std::cout << log_entry.dump() << std::endl;
    }
}
