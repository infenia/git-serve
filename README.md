# Git Serve

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Overview

## Overview

Junkyard provides REST and WebSocket interfaces for managing and querying a SQL database. It is designed for high
performance and observability, supporting structured logging and containerized deployment.

## Architecture

- **REST API**: Handles HTTP requests for CRUD operations.
- **WebSocket**: Supports real-time data communication.
- **Modular Source**: Key modules include `rest/`, `utils/`, `easymarry/`, and `command/`.
- **Logging**: Structured JSON logs with traceability.

## Setup

1. **Clone the repository**
2. **Install dependencies** (see Dockerfile for build requirements)
3. **Configure environment**
    - Copy `.env.example` to `.env` and set your secrets/variables.

## Usage

- Build and run locally using CMake or Docker (recommended).
- REST endpoints and WebSocket usage: see code comments in `src/rest/` and `src/easymarry/`.

## Docker Instructions

- Build: `./scripts/docker-build-push.sh`
- Run: `docker run --env-file .env -p 8080:8080 data-app:latest`
- Multi-stage build ensures minimal runtime image.

## Code Quality

- Run `clang-format` and `clang-tidy` before committing:
    - `find src -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i`
    - `clang-tidy src/**/*.cpp -- -Isrc`
- Use the pre-commit script: `scripts/pre-commit-format-lint.sh`

## Logging

- All HTTP requests/responses (except /health) are logged in structured JSON.
- For log rotation/retention, use external tools (e.g., logrotate, Docker log drivers).

## Error Handling

- External calls (e.g., GitHub API) have retry logic and error logging.
- See comments in `src/easymarry/easymarry.cpp` for robust error handling patterns.

## License

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at:

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

## Contribution Guidelines

This project is maintained by [Infenia Private Limited](https://www.infenia.in/). We welcome contributions from the community.

- Fork and branch from `main`.
- Run format/lint and ensure no warnings.
- Add/expand docstrings for new modules/functions.
- Submit PRs with clear descriptions.

## License

See `LICENSE` file.
