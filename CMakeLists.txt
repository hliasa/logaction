cmake_minimum_required(VERSION 2.8)

project(logaction)
add_executable(${PROJECT_NAME} "main.cpp" "log.cpp" "log.h")
link_directories(/usr/lib)
target_link_libraries(${PROJECT_NAME} sqlite3)
