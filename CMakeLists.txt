# Minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ChatApp VERSION 1.0)

# C Standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# cJSON library
# Add the cJSON library to the build by specifying its source file
add_library(cjson STATIC lib/cjson/cJSON.c)
target_include_directories(cjson PUBLIC lib/cjson)

# Chat Actions (shared code between client and server)
# This code is in 'common' and will be linked by both the client and the server
add_library(chat_actions STATIC src/common/chat_actions.c)
target_include_directories(chat_actions PUBLIC src/common/include src/server/include lib/cjson)

add_library(client_handler STATIC src/server/client_handler.c)
target_include_directories(client_handler PUBLIC src/server/include)

# Server build
# The server source files are in the 'server' folder
add_executable(server_app src/server/server.c)

# Link the necessary libraries to the server executable
target_link_libraries(server_app cjson chat_actions client_handler pthread)

# Client build
# The client source files are in the 'client' folder
add_executable(client_app src/client/client.c)

# Link the necessary libraries to the client executable
target_link_libraries(client_app cjson chat_actions pthread)

# Installation instructions (optional)
# You can add these if you want CMake to handle installation paths for the executables
install(TARGETS client_app server_app DESTINATION bin)

# Documentation and output path settings
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Print summary
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Project: ${PROJECT_NAME} - Version ${PROJECT_VERSION}")

