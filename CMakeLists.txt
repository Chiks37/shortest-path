cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_NAME shortest_path)
project(${PROJECT_NAME})

# -- Target selection (set by build.sh via -D flags)
option(BUILD_CUSTOM_TEST_NETWORKIT "Build custom_test_networkit executable (networkit reference)"
       OFF)
option(BUILD_CUSTOM_TEST_GAPBS "Build custom_test_gapbs executable (GAPBS reference)" OFF)
option(BUILD_LONGEST_PATH_SEARCH "Build longest_path_search executable" OFF)
option(BUILD_GTEST "Build and run Google Tests" OFF)
option(BUILD_BENCHMARK "Build benchmark executable" OFF)

# -- Build configuration
set(CMAKE_CONFIGURATION_TYPES
    "Debug;Release"
    CACHE STRING "Configs" FORCE)
if(GDB_BUILD)
    set(CMAKE_BUILD_TYPE Debug)
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
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
    # gapbs_launcher.cpp uses GAPBS headers; compiled separately via cmake/gapbs/
    list(
        FILTER
        SOURCES_TEMP
        EXCLUDE
        REGEX
        ".*gapbs.*\\.cpp$")
    set(LIB_SOURCES ${LIB_SOURCES} ${SOURCES_TEMP})
endforeach()
set(HEADERS_FOLDERS ${HEADERS_FOLDERS} "${LIB_FOLDER}") # general.hpp
# --

# -- Prebuilt 3rdparty dependencies networkit/TBB/googletest are built once per configuration into
# cache/deps/<name>/<config>/ and imported here. They are rebuilt only when the artifact is missing
# (editing this file never rebuilds them), and debug/release versions are kept and built
# independently.
set(DEPS_CACHE_DIR "${CMAKE_SOURCE_DIR}/cache/deps")
string(TOLOWER "${CMAKE_BUILD_TYPE}" DEP_CONFIG_DIR)
include(${CMAKE_SOURCE_DIR}/cmake/PrebuiltDeps.cmake)

find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

# networkit (shared, monolithic). tlx is statically baked into the .so.
ensure_prebuilt_dep(
    NAME
    networkit
    SRC
    "${CMAKE_SOURCE_DIR}/3rdparty/networkit"
    LIB_GLOB
    "libnetworkit.so"
    OUT_VAR
    NETWORKIT_LIB
    CMAKE_ARGS
    -DNETWORKIT_BUILD_CORE=ON
    -DNETWORKIT_BUILD_TESTS=OFF
    -DNETWORKIT_MONOLITH=ON
    -DNETWORKIT_BUILD_PYTHON=OFF)
add_library(networkit SHARED IMPORTED)
set_target_properties(
    networkit
    PROPERTIES
        IMPORTED_LOCATION "${NETWORKIT_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/3rdparty/networkit/include;${CMAKE_SOURCE_DIR}/3rdparty/networkit/extlibs/tlx;${CMAKE_SOURCE_DIR}/3rdparty/networkit/extlibs/ttmath"
)

# oneTBB (shared). Output lands in a compiler/arch-specific subdir, so glob. Debug builds emit
# libtbb_debug.so instead of libtbb.so.
ensure_prebuilt_dep(
    NAME
    tbb
    SRC
    "${CMAKE_SOURCE_DIR}/3rdparty/oneTBB"
    LIB_GLOB
    "libtbb.so"
    "libtbb_debug.so"
    OUT_VAR
    TBB_LIB
    CMAKE_ARGS
    -DTBB_TEST=OFF
    -DTBB_EXAMPLES=OFF)
add_library(TBB::tbb SHARED IMPORTED)
set_target_properties(
    TBB::tbb PROPERTIES IMPORTED_LOCATION "${TBB_LIB}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/3rdparty/oneTBB/include")

if(GDB_BUILD)
    add_compile_options(-O0 -g3 -ggdb)
    set(GDB_MESSAGE "enabled")
else()
    set(GDB_MESSAGE "disabled")
endif()

# -- Shared static library (avoids recompiling lib sources for every target)
add_library(sp_lib STATIC ${LIB_SOURCES})
target_include_directories(sp_lib PUBLIC ${HEADERS_FOLDERS})
target_link_libraries(sp_lib PUBLIC networkit OpenMP::OpenMP_CXX TBB::tbb Boost::headers)
# Runtime ON/OFF config consumed by AlgoConfig (edited without rebuilding).
target_compile_definitions(sp_lib PRIVATE ALGOS_CONFIG_PATH="${CMAKE_SOURCE_DIR}/algorithms.conf")

# -- Executable targets
if(BUILD_CUSTOM_TEST_NETWORKIT)
    add_executable(custom_test_networkit app/custom_test_networkit/main.cpp)
    target_link_libraries(custom_test_networkit PRIVATE sp_lib)
endif()

if(BUILD_CUSTOM_TEST_GAPBS OR BUILD_BENCHMARK)
    # Mirrors how networkit/TBB are handled: compiled into cache/deps/gapbs/ so the artifact
    # survives ./build.sh -c.
    add_subdirectory(cmake/gapbs ${DEPS_CACHE_DIR}/gapbs EXCLUDE_FROM_ALL)
endif()

if(BUILD_CUSTOM_TEST_GAPBS)
    add_executable(custom_test_gapbs app/custom_test_gapbs/main.cpp)
    target_link_libraries(custom_test_gapbs PRIVATE sp_lib gapbs_launcher)
endif()

if(BUILD_LONGEST_PATH_SEARCH)
    add_executable(longest_path_search app/longest_path_search/main.cpp)
    target_link_libraries(longest_path_search PRIVATE sp_lib)
endif()

if(BUILD_BENCHMARK)
    add_executable(benchmark app/benchmark/main.cpp)
    target_link_libraries(benchmark PRIVATE sp_lib gapbs_launcher)
endif()

if(BUILD_GTEST)
    ensure_prebuilt_dep(
        NAME
        googletest
        SRC
        "${CMAKE_SOURCE_DIR}/3rdparty/networkit/extlibs/googletest/googletest"
        LIB_GLOB
        "libgtest.a"
        OUT_VAR
        GTEST_LIB
        CMAKE_ARGS
        -DGOOGLETEST_VERSION=1.8.0)
    get_filename_component(GTEST_LIB_DIR "${GTEST_LIB}" DIRECTORY)

    add_library(gtest STATIC IMPORTED)
    set_target_properties(
        gtest
        PROPERTIES IMPORTED_LOCATION "${GTEST_LIB}"
                   INTERFACE_INCLUDE_DIRECTORIES
                   "${CMAKE_SOURCE_DIR}/3rdparty/networkit/extlibs/googletest/googletest/include"
                   INTERFACE_LINK_LIBRARIES Threads::Threads)

    add_library(gtest_main STATIC IMPORTED)
    set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION "${GTEST_LIB_DIR}/libgtest_main.a"
                                                INTERFACE_LINK_LIBRARIES gtest)

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
message(STATUS "   Configuration:              ${CMAKE_BUILD_TYPE}")
message(STATUS "   Compiler ID:                ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "   GDB build:                  ${GDB_MESSAGE}")
message(STATUS "   custom_test_networkit:      ${BUILD_CUSTOM_TEST_NETWORKIT}")
message(STATUS "   custom_test_gapbs:          ${BUILD_CUSTOM_TEST_GAPBS}")
message(STATUS "   longest_path_search:        ${BUILD_LONGEST_PATH_SEARCH}")
message(STATUS "   sp_tests (gtest):           ${BUILD_GTEST}")
message(STATUS "   benchmark:                  ${BUILD_BENCHMARK}")
message(STATUS "")
message(STATUS "======================================")
