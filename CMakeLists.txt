cmake_minimum_required(VERSION 3.5)

project(key_switcher)
set (CMAKE_CXX_STANDARD 11)
add_executable(${PROJECT_NAME} "main.cpp" "proc_thread_list.h")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} psapi)
