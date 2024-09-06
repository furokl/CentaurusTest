cmake_minimum_required(VERSION 3.26)

project(Utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SOURCES 
	"ConfigReader/*.h"
	"ConfigReader/*.cpp"
	"*.cpp"
	"*.h"
)

add_library(Utils STATIC ${SOURCES})

target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ConfigReader)
target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})