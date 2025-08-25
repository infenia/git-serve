# ---- Stage 1: Build Stage ----
# Uses a minimal Debian image to build the application
FROM debian:bookworm-slim AS builder

# Install required build dependencies
RUN apt-get update && apt-get install -y \
    build-essential pkg-config git \
    libwebsockets-dev libcurl4-gnutls-dev g++ curl cmake make \
    libspdlog-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory for build
WORKDIR /easymarry-data

# Copy source code
COPY . .

# Create build directory
WORKDIR /easymarry-data/build

# Clean build directory (proactive)
RUN rm -rf /easymarry-data/build/* || true

# Build the application using CMake and strip binary for minimal size
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc) && strip em-data

# ---- Stage 2: Runtime Stage ----
# Uses a minimal Debian image for runtime
FROM debian:bookworm-slim AS runtime

# Install only necessary runtime libraries
RUN apt-get update && apt-get install -y \
    libwebsockets17 libcurl4-gnutls-dev libspdlog-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /easymarry-data

# Copy only the compiled binary from the build stage
COPY --from=builder /easymarry-data/build/em-data .

# Ensure the binary has execution permission
RUN chmod +x em-data

# Expose default port
EXPOSE 8080

# Start the application
CMD ["./em-data"]