cmake_minimum_required(VERSION 3.10)
project(DeribitTrading)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
)

# Add executable
add_executable(deribit_trading
    src/main.cpp
    src/deribit_api.cpp
    src/config.cpp
    src/orderbook.cpp
)

# Link libraries
target_link_libraries(deribit_trading
    OpenSSL::SSL
    OpenSSL::Crypto
    crypto
    ssl
    Threads::Threads
    CURL::libcurl
    nlohmann_json::nlohmann_json
    CURL::libcurl
    jsoncpp
)

# Copy config file to build directory
configure_file(${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY) 