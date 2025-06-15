<div align="center">

# 🚀 Git Serve

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](https://github.com/infenia/git-serve/pulls)
[![GitHub stars](https://img.shields.io/github/stars/infenia/git-serve?style=social)](https://github.com/infenia/git-serve/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/infenia/git-serve?style=social)](https://github.com/infenia/git-serve/network/members)
[![Build Status](https://github.com/infenia/git-serve/actions/workflows/ci.yml/badge.svg)](https://github.com/infenia/git-serve/actions)
[![codecov](https://codecov.io/gh/infenia/git-serve/graph/badge.svg)](https://codecov.io/gh/infenia/git-serve)

*A high-performance Git HTTP server written in modern C++*

[📖 Documentation](#documentation) •
[🚀 Quick Start](#quick-start) •
[💡 Features](#-features) •
[🤝 Contributing](#-contributing)

</div>

## 🎯 Overview

Git Serve is an open-source, high-performance HTTP server implementation designed for serving files from git repositories with a focus on performance, security, and developer experience. Built with modern C++17 and leveraging industry-standard libraries, it provides a robust foundation for git repository hosting with a clean REST API.

## ✨ Features

🚀 **High Performance**
- Built with lock-free data structures for maximum concurrency
- Non-blocking I/O operations for efficient resource usage
- Optimized for high throughput and low latency

🔒 **Secure by Design**
- Input validation on all endpoints
- Secure defaults and best practices
- Regular security updates and dependency management

📊 **Observability**
- Structured JSON logging with request tracing
- Built-in metrics and health checks
- Easy integration with monitoring tools

🐳 **Developer Friendly**
- Container-first approach with Docker support
- Comprehensive documentation and examples
- Modern C++ codebase with clean architecture

- **RESTful API**: HTTP endpoints for git operations
- **High Performance**: Built with lock-free data structures and efficient I/O handling
- **Structured Logging**: JSON-formatted logs with request tracing
- **Container Ready**: Docker support for easy deployment
- **Thread-safe**: Designed for concurrent request handling

## Project Structure

```
src/
├── easymarry/     # Core git operations and protocol handling
├── rest/          # REST API implementation
│   ├── http_transaction.hpp  # HTTP request/response handling
│   ├── io_handler.hpp       # I/O operations
│   └── router.{hpp,cpp}     # Request routing
├── utils/         # Utility libraries
│   ├── logger.{hpp,cpp}     # Structured logging
│   ├── lock_free_queue.hpp  # Thread-safe queue
│   └── trace_id.{hpp,cpp}   # Request tracing
└── main.cpp       # Application entry point
```

## 🚀 Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 9+, Clang 10+)
- CMake 3.15+
- libwebsockets-dev
- spdlog
- nlohmann-json

### Building from Source

```bash
# Clone the repository
git clone https://github.com/infenia/git-serve.git
cd git-serve

# Configure and build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Run the server
./git-serve
```

### Using Docker (Recommended)

```bash
# Pull the latest image
docker pull ghcr.io/infenia/git-serve:latest

# Run with default settings
docker run -d -p 8080:8080 --name git-serve ghcr.io/infenia/git-serve:latest
```

## 📚 Documentation

For detailed documentation, please visit our [documentation website](https://infenia.github.io/git-serve/).

- [API Reference](https://infenia.github.io/git-serve/api/)
- [Configuration Guide](https://infenia.github.io/git-serve/configuration/)
- [Deployment](https://infenia.github.io/git-serve/deployment/)
- [Contributing Guide](https://infenia.github.io/git-serve/contributing/)

## Building

### Using CMake

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Using Docker

```bash
# Build the container
docker build -t git-serve .

# Run the server
docker run -p 8080:8080 git-serve
```

## Configuration

Environment variables:
- `PORT`: Server port (default: 8080)
- `LOG_LEVEL`: Logging level (default: info)
- `GIT_REPO_PATH`: Path to git repositories (default: ./repos)

## API Endpoints

### Repository Operations

- `GET /repos` - List available repositories
- `GET /repos/{repo}/info/refs` - Get repository references
- `POST /repos/{repo}/git-upload-pack` - Handle git upload pack
- `POST /repos/{repo}/git-receive-pack` - Handle git receive pack

## Logging

The server uses structured JSON logging with the following fields:
- `timestamp`: ISO 8601 timestamp
- `level`: Log level (info, error, etc.)
- `message`: Log message
- `trace_id`: Unique request identifier

Example log entry:
```json
{
  "timestamp": "2025-06-15T16:45:22.123Z",
  "level": "info",
  "message": "Request received",
  "method": "GET",
  "path": "/repos/example/info/refs"
}
```

## Performance

- Uses lock-free queue for concurrent request handling
- Non-blocking I/O operations
- Efficient memory management
- Connection pooling for database operations

## Code Quality

- Follows Google C++ Style Guide
- Uses clang-format and clang-tidy for code style and static analysis
- Unit tests (TODO)
- Integration tests (TODO)

## 🤝 Contributing

We love contributions from the community! Here's how you can help:

### Ways to Contribute

- 🐛 Report bugs by [opening an issue](https://github.com/infenia/git-serve/issues/new?template=bug_report.md)
- 💡 Suggest new features or improvements
- 📝 Improve documentation
- 🛠 Fix bugs or implement new features
- 🧪 Write tests
- 🔍 Review pull requests

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run tests and linters (see below)
5. Commit your changes (`git commit -m 'Add some amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Code Style & Quality

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some exceptions.

Before submitting a PR, please run:

```bash
# Format code
./scripts/format.sh

# Run linters
./scripts/lint.sh

# Run tests
./scripts/test.sh
```

### Commit Message Format

We use [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

Example:
```
feat(api): add user authentication endpoint

- Add POST /auth/login endpoint
- Implement JWT token generation
- Add tests for authentication flow

Closes #123
```

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for the full license text.

## Security

- All HTTP endpoints should use HTTPS in production
- Input validation is performed on all requests
- Sensitive information is not logged

## 📬 Support

### Community Support

- [GitHub Discussions](https://github.com/infenia/git-serve/discussions) - Ask questions and share ideas
- [Discord Chat](https://discord.gg/your-invite) - Join our community
- [Twitter](https://twitter.com/your-handle) - Follow us for updates

### Professional Support

For enterprise support, SLA, and consulting services, please contact [support@infenia.com](mailto:support@infenia.com).

## 🤝 Acknowledgements

We're grateful to these amazing projects that make Git Serve possible:

- [libwebsockets](https://libwebsockets.org/) - Lightweight C library for websockets
- [spdlog](https://github.com/gabime/spdlog) - Fast C++ logging library
- [nlohmann-json](https://github.com/nlohmann/json) - JSON for Modern C++
- [cpr](https://github.com/libcpr/cpr) - C++ Requests: Curl for People
- [fmt](https://github.com/fmtlib/fmt) - A modern formatting library

## 🌟 Stargazers

[![Stargazers repo roster for @infenia/git-serve](https://reporoster.com/stars/infenia/git-serve)](https://github.com/infenia/git-serve/stargazers)

## 👥 Contributors

A big thank you to all the amazing people who have contributed to Git Serve! 🎉

<div align="center">
  <div style="display: flex; flex-wrap: wrap; justify-content: center; gap: 2rem; margin: 2rem 0;">
    <div style="text-align: center;">
      <a href="https://github.com/LostWarning" style="text-decoration: none;">
        <img 
          src="https://github.com/LostWarning.png?size=150" 
          width="120" 
          height="120"
          style="border-radius: 50%; border: 3px solid #e74c3c; transition: transform 0.3s ease;"
          onmouseover="this.style.transform='scale(1.1)'"
          onmouseout="this.style.transform='scale(1)'"
          alt="LostWarning"
        />
        <h3 style="margin: 0.5rem 0 0.2rem 0; color: #2c3e50;">LostWarning</h3>
        <div style="display: flex; justify-content: center; gap: 0.5rem; margin-bottom: 0.5rem;">
          <img 
            src="https://img.shields.io/badge/Contributor-e74c3c?style=flat-square" 
            alt="Contributor"
          />
          <img 
            src="https://img.shields.io/badge/Code-3498db?style=flat-square" 
            alt="Code"
          />
        </div>
      </a>
      <div style="display: flex; justify-content: center; gap: 0.5rem;">
        <a href="https://github.com/LostWarning" title="GitHub">
          <img src="https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white" height="24" alt="GitHub">
        </a>
      </div>
    </div>
    <div style="text-align: center;">
      <a href="https://github.com/arun-infenia" style="text-decoration: none;">
        <img 
          src="https://github.com/arun-infenia.png?size=150" 
          width="120" 
          height="120"
          style="border-radius: 50%; border: 3px solid #2ecc71; transition: transform 0.3s ease;"
          onmouseover="this.style.transform='scale(1.1)'"
          onmouseout="this.style.transform='scale(1)'"
          alt="Arun"
        />
        <h3 style="margin: 0.5rem 0 0.2rem 0; color: #2c3e50;">Arun</h3>
        <div style="display: flex; justify-content: center; gap: 0.5rem; margin-bottom: 0.5rem;">
          <img 
            src="https://img.shields.io/badge/Contributor-2ecc71?style=flat-square" 
            alt="Contributor"
          />
          <img 
            src="https://img.shields.io/badge/Code-3498db?style=flat-square" 
            alt="Code"
          />
        </div>
      </a>
      <div style="display: flex; justify-content: center; gap: 0.5rem;">
        <a href="https://github.com/arun-infenia" title="GitHub">
          <img src="https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white" height="24" alt="GitHub">
        </a>
      </div>
    </div>
  </div>
  
  <p style="margin-top: 2rem; font-style: italic;">
    Want to see your name here? Check out our <a href="#-contributing">contributing guide</a>!
  </p>
</div>

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## 🙏 Thanks

Thank you to all our contributors and users! Your feedback and contributions make this project better every day.

---

<div>
  Made with ❤️ by <a href="https://www.infenia.com">Infenia</a>
</div>
