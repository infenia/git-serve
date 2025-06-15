cmake_minimum_required(VERSION 3.19.0)
project(
  em-data
  VERSION 0.1.0
  LANGUAGES C CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)

# Export compile_commands.json for auto completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find_package(Liburing REQUIRED)

# Add spdlog and nlohmann_json (for JSON logging)
find_package(spdlog REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Define build for JunkYard
set(EM_MASTERDATA_SOURCE
    src/easymarry/easymarry.cpp
    src/rest/connection.cpp
    src/rest/http_transaction.cpp
    src/rest/io_handler.cpp
    src/rest/router.cpp
    src/main.cpp
    src/utils/logger.cpp
    src/utils/trace_id.cpp
    )
add_executable(${PROJECT_NAME} ${EM_MASTERDATA_SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE websockets curl z
                                              ${LIBWEBSOCKETS_DEP_LIBS}
                                              spdlog::spdlog nlohmann_json::nlohmann_json)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

configure_file("${PROJECT_SOURCE_DIR}/src/build/version.h.in"
               "${PROJECT_SOURCE_DIR}/src/build/version.h")
