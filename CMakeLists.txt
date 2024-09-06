cmake_minimum_required(VERSION 3.26)

set(PROJECT_NAME Centaurus)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(Utils)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/server.properties DESTINATION ${CMAKE_CURRENT_BINARY_DIR})