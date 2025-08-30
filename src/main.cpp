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

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#include "git_serve/git_serve.hpp"
#include "git_serve/master_data.hpp"
#include "utils/logger.hpp"

// Global interrupt flag for signal handling
std::atomic_bool g_interrupted(false);

// Signal handler for SIGINT and SIGTERM
void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    logger::get()->info("Received interrupt signal, shutting down gracefully");
    g_interrupted.store(true);
  }
}

int main(int argc, const char **argv) {
  logger::init();
  logger::get()->info("Application starting");

  // Set up signal handlers for graceful shutdown
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  Configuration conf;
  conf.populate_config();

  GitServe em(conf, g_interrupted);
  em.start_timer_job();

  // Wait for interrupt signal
  while (!g_interrupted.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  logger::get()->info("Application shutting down");
  em.shutdown();
  logger::get()->info("Application shutdown complete");
  return 0;
}