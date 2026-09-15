cmake_minimum_required(VERSION 3.10)

project(txtco)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app)

add_subdirectory("command")
add_subdirectory("command_txtco")
add_subdirectory("command_help")
add_subdirectory("my_lib\\my_ref_dict")

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    "command_lib"
    "my_ref_dict_lib"
    "command_txtco_lib"
    "command_help_lib"
)

if(WIN32)
    target_link_libraries(txtco PRIVATE user32)
endif()