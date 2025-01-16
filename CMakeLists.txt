cmake_minimum_required(VERSION 3.10)
project(DeribitTrading)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

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
)

# Link libraries
target_link_libraries(deribit_trading
    PRIVATE
    CURL::libcurl
    jsoncpp
)

# Copy config file to build directory
configure_file(${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY) 