cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_NAME shortest_path)
project(${PROJECT_NAME})

# -- Target selection (set by build.sh via -D flags)
option(BUILD_CUSTOM_TEST "Build custom_test executable" OFF)
option(BUILD_LONGEST_PATH_SEARCH "Build longest_path_search executable" OFF)
option(BUILD_GTEST "Build and run Google Tests" OFF)

# -- Build configuration
set(CMAKE_CONFIGURATION_TYPES
    "Debug;Release"
    CACHE STRING "Configs" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/")

# -- Collect library sources and include paths
set(THIRD_PARTY_FOLDER "3rdparty")
set(THIRD_PARTY_NAMES "graphio")
foreach(LIB_NAME ${THIRD_PARTY_NAMES})
    set(THIRD_PARTY_CODE_FOLDERS "${THIRD_PARTY_CODE_FOLDERS}" "${THIRD_PARTY_FOLDER}/${LIB_NAME}")
endforeach()

set(LIB_FOLDER "lib")
set(LIB_NAMES "search_algos" "launcher" "tracked_pq")
foreach(LIB_NAME ${LIB_NAMES})
    set(LIBS_CODE_FOLDERS "${LIBS_CODE_FOLDERS}" "${LIB_FOLDER}/${LIB_NAME}")
endforeach()

set(CODE_FOLDERS ${THIRD_PARTY_CODE_FOLDERS} ${LIBS_CODE_FOLDERS})

foreach(FOLDER ${CODE_FOLDERS})
    set(HEADERS_FOLDERS ${HEADERS_FOLDERS} "${FOLDER}/inc/")

    file(
        GLOB_RECURSE
        SOURCES_TEMP
        CONFIGURE_DEPENDS
        "${FOLDER}/*.c"
        "${FOLDER}/*.cpp"
        "${FOLDER}/**/*.c"
        "${FOLDER}/**/*.cpp")
    set(LIB_SOURCES ${LIB_SOURCES} ${SOURCES_TEMP})
endforeach()
set(HEADERS_FOLDERS ${HEADERS_FOLDERS} "${LIB_FOLDER}") # general.hpp
# --

# -- Third-party build flags
set(NETWORKIT_BUILD_CORE
    ON
    CACHE BOOL "" FORCE)
set(NETWORKIT_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(NETWORKIT_BUILD_PYTHON
    OFF
    CACHE BOOL "" FORCE)
set(TBB_TEST
    OFF
    CACHE BOOL "" FORCE)
set(TBB_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)

# 3rdparty libs are built into cache/deps/ so they survive a clean of build/.
set(DEPS_CACHE_DIR "${CMAKE_SOURCE_DIR}/cache/deps")

add_subdirectory(3rdparty/networkit ${DEPS_CACHE_DIR}/networkit)
add_subdirectory(3rdparty/oneTBB ${DEPS_CACHE_DIR}/tbb)

find_package(OpenMP REQUIRED)

if(GDB_BUILD)
    add_compile_options(-O0 -g3 -ggdb)
    set(GDB_MESSAGE "enabled")
else()
    set(GDB_MESSAGE "disabled")
endif()

# -- Shared static library (avoids recompiling lib sources for every target)
add_library(sp_lib STATIC ${LIB_SOURCES})
target_include_directories(sp_lib PUBLIC ${HEADERS_FOLDERS})
target_link_libraries(sp_lib PUBLIC networkit OpenMP::OpenMP_CXX TBB::tbb)

# -- Executable targets
if(BUILD_CUSTOM_TEST)
    add_executable(custom_test app/custom_test/main.cpp)
    target_link_libraries(custom_test PRIVATE sp_lib)
endif()

if(BUILD_LONGEST_PATH_SEARCH)
    add_executable(longest_path_search app/longest_path_search/main.cpp)
    target_link_libraries(longest_path_search PRIVATE sp_lib)
endif()

if(BUILD_GTEST)
    # GOOGLETEST_VERSION must be set before add_subdirectory: the bundled CMakeLists uses it in
    # project() and set_target_properties(VERSION).
    set(GOOGLETEST_VERSION "1.8.0")
    add_subdirectory(3rdparty/networkit/extlibs/googletest/googletest ${DEPS_CACHE_DIR}/googletest
                     EXCLUDE_FROM_ALL)

    add_executable(sp_tests app/gtest/algo_correctness_test.cpp)
    target_link_libraries(sp_tests PRIVATE sp_lib gtest_main)
    target_compile_definitions(
        sp_tests
        PRIVATE TEST_GRAPH_PATH="${CMAKE_SOURCE_DIR}/graphs/UNN/Nizhny_Novgorod/Nizhny_Novgorod.mtx"
    )

    add_custom_command(
        TARGET sp_tests
        POST_BUILD
        COMMAND sh -c "$<TARGET_FILE:sp_tests> || true"
        COMMENT "Running sp_tests..."
        VERBATIM)
endif()

# -- Configuration summary
message(STATUS "")
message(STATUS "======================================")
message(STATUS "General configuration for ${PROJECT_NAME} project:")
message(STATUS "")
message(STATUS "   Configuration:           ${CMAKE_BUILD_TYPE}")
message(STATUS "   Compiler ID:             ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "   GDB build:               ${GDB_MESSAGE}")
message(STATUS "   custom_test:             ${BUILD_CUSTOM_TEST}")
message(STATUS "   longest_path_search:     ${BUILD_LONGEST_PATH_SEARCH}")
message(STATUS "   sp_tests (gtest):        ${BUILD_GTEST}")
message(STATUS "")
message(STATUS "======================================")
