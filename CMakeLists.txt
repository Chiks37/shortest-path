cmake_minimum_required(VERSION 2.8)

message( STATUS "x${CMAKE_CXX_COMPILER_ID}")

set(PROJECT_NAME shortest_path)
project(${PROJECT_NAME})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/")

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")

set(CONDA_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/env")
link_directories("${CONDA_PREFIX}/lib")

set(
  includes
  "${includes}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/graphIO"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/ttmath"
  "${CONDA_PREFIX}/include"
  )
include_directories(${includes})

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} main.cpp)
target_link_libraries(${PROJECT_NAME} networkit)

# REPORT
message( STATUS "")
message( STATUS "General configuration for ${PROJECT_NAME}")
message( STATUS "======================================")
message( STATUS "")
message( STATUS "   Configuration: ${CMAKE_BUILD_TYPE}")
message( STATUS "")