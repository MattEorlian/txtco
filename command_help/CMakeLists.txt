cmake_minimum_required(VERSION 3.10)

set(LIB_NAME "command_help_lib")

add_library(${LIB_NAME} STATIC "command_help.cpp")

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIB_NAME} PUBLIC "command_lib")