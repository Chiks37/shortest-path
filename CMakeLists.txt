cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_NAME shortest_path)
project(${PROJECT_NAME})

# Setting configuration
set(CMAKE_CONFIGURATION_TYPES
    "Debug;Release"
    CACHE STRING "Configs" FORCE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/")

# -- Collecting all headers folders and source files
set(THIRD_PARTY_FOLDER "3rdparty") # Name of folder, containing third-party
                                   # libraries
set(THIRD_PARTY_NAMES "graphio") # List here libs names (names of libs folder in 3rd
                          # party folder)
foreach(LIB_NAME ${THIRD_PARTY_NAMES})
  set(THIRD_PARTY_CODE_FOLDERS "${THIRD_PARTY_CODE_FOLDERS}"
                        "${THIRD_PARTY_FOLDER}/${LIB_NAME}")
endforeach()

set(LIB_FOLDER "lib") # Name of folder, containing third-party
                                   # libraries
set(LIB_NAMES "launcher") # List here libs names (names of libs folder in 3rd
                          # party folder)
foreach(LIB_NAME ${LIB_NAMES})
  set(LIBS_CODE_FOLDERS "${LIBS_CODE_FOLDERS}"
                        "${LIB_FOLDER}/${LIB_NAME}")
endforeach()

set(PROJECT_CODE_FOLDER "search_algos")

set(CODE_FOLDERS ${THIRD_PARTY_CODE_FOLDERS} ${LIBS_CODE_FOLDERS} ${PROJECT_CODE_FOLDER})

foreach(FOLDER ${CODE_FOLDERS})
  set(HEADERS_FOLDERS ${HEADERS_FOLDERS} "${FOLDER}/inc/")

  file(GLOB_RECURSE SOURCES_TEMP CONFIGURE_DEPENDS "${FOLDER}/*.c" "${FOLDER}/*.cpp"
       "${FOLDER}/**/*.c" "${FOLDER}/**/*.cpp")
  set(SOURCES ${SOURCES} ${SOURCES_TEMP})
endforeach()
# --

# Включаем сборку самой C++ библиотеки
set(NETWORKIT_BUILD_CORE ON CACHE BOOL "" FORCE)
# Выключаем тесты и прочий хлам, чтобы не ждать вечность
set(NETWORKIT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NETWORKIT_BUILD_PYTHON OFF CACHE BOOL "" FORCE)

# Provide headers and sources
include_directories(${HEADERS_FOLDERS})
add_subdirectory(3rdparty/networkit)

if(GDB_BUILD)
  add_compile_options(-O0 -g3 -ggdb)
  set(GDB_MESSAGE "enabled")
else()
  set(GDB_MESSAGE "disabled")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE networkit)

# General project config output
message(STATUS "")
message(STATUS "======================================")
message(STATUS "General configuration for ${PROJECT_NAME} project:")
message(STATUS "")
message(STATUS "   Configuration: ${CMAKE_BUILD_TYPE}")
message(STATUS "   Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "   GDB build: ${GDB_MESSAGE}")
message(STATUS "")
message(STATUS "======================================")
