cmake_minimum_required(VERSION 3.26)

set(PROJECT_NAME Centaurus)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CONFIG_PATH "${CMAKE_SOURCE_DIR}/server.properties")
set(CONTENT_PATH "${CMAKE_SOURCE_DIR}")

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(Utils)