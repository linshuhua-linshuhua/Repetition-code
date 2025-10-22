cmake_minimum_required(VERSION 3.16)
project(YinshanMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system json)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Add executable
add_executable(yinshan_monitor
    main.cpp
    yinshan_monitor.cpp
)

# Link libraries
target_link_libraries(yinshan_monitor
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

# Compiler flags
target_compile_options(yinshan_monitor PRIVATE
    -Wall -Wextra -Wpedantic
    -O2
)

# Define preprocessor macros
target_compile_definitions(yinshan_monitor PRIVATE
    BOOST_BEAST_USE_STD_STRING_VIEW
)