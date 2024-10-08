cmake_minimum_required(VERSION 3.26)

set(PROJECT_NAME Centaurus-Client)
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/APP_ICON.rc")
get_filename_component(APP_ICON_ABSOLUTE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/APP_ICON.ico" ABSOLUTE)

project(${PROJECT_NAME} CXX)
file(GLOB SOURCES 
	"sources/*.h" 
	"sources/*.cpp"
)
add_executable(${PROJECT_NAME} 
	${SOURCES} 
	${APP_ICON}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_PATH="${CONFIG_PATH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE CONTENT_PATH="${CONTENT_PATH}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_GLOBAL_ApplicationIcon "${APP_ICON_ABSOLUTE_PATH}")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Utils)