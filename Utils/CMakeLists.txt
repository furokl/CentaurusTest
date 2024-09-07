cmake_minimum_required(VERSION 3.26)

project(Utils)

file(GLOB SOURCES 
	"ConfigReader/*.h"
	"ConfigReader/*.cpp"
	"*.cpp"
	"*.h"
)

add_library(Utils STATIC ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ConfigReader)
target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})