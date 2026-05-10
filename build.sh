#!/bin/bash

BUILD_DIR="./build"
DEPS_CACHE_DIR="./cache/deps"

function do_build() {
    local custom_test="$1"
    local longest="$2"
    local gtest="$3"
    local gdb="$4"

    mkdir -p build
    cd build \
    && cmake \
        -DGDB_BUILD="$gdb" \
        -DBUILD_CUSTOM_TEST="$custom_test" \
        -DBUILD_LONGEST_PATH_SEARCH="$longest" \
        -DBUILD_GTEST="$gtest" \
        .. \
    && cmake --build .
}

function do_clang_format() {
    find ./app/ ./lib/ -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +
}

function do_cmake_format() {
    cmake-format -i "CMakeLists.txt"
}

# Removes build/ and bin/ only; cache/deps/ (3rdparty artifacts) is preserved.
function clean() {
    rm -rf "$BUILD_DIR"
    rm -rf bin
}

# Removes the 3rdparty build cache (networkit, TBB, googletest).
# Use when you need a truly fresh build of everything.
function clean_cache() {
    rm -rf "$DEPS_CACHE_DIR"
}

function help() {
    echo "Usage: $0 [TARGET] [-gdb]"
    echo "       $0 -c | -clf | -cmf | -h"
    echo ""
    echo "Build targets (default: -a):"
    echo "  -a,  --all              Build all targets and run Google Tests"
    echo "  -A,  --all-but-gtest    Build all targets, skip Google Tests"
    echo "  -t,  --custom-test      Build custom_test only (interactive algo comparison)"
    echo "  -l,  --longest          Build longest_path_search only (long-route finder)"
    echo "  -g,  --gtest            Build and run Google Tests only"
    echo ""
    echo "Build modifiers:"
    echo "  -gdb                    Include full debug symbols (-O0 -g3 -ggdb)"
    echo ""
    echo "Other commands:"
    echo "  -c,  --clean            Remove build/ and bin/ (cache/deps/ is preserved)"
    echo "  -cb, --clean-build      Execute --clean then --all"
    echo "  -cc, --clean-cache      Remove cache/deps/ (3rdparty build artifacts)"
    echo "  -clf,--clang-format     Format C/C++ files using clang-format"
    echo "  -cmf,--cmake-format     Format CMakeLists.txt using cmake-format"
    echo "  -h,  --help             Print this help"
    echo ""
    echo "Examples:"
    echo "  $0                    # build all (same as -a)"
    echo "  $0 -t -gdb            # build custom_test with debug symbols"
    echo "  $0 -g                 # build and run Google Tests"
    echo "  $0 -cb                # clean then build all (3rdparty cache preserved)"
    echo "  $0 -cc && $0 -a       # full rebuild including 3rdparty"
}

# Parse TARGET and -gdb independently so their order does not matter.
TARGET=""
GDB_BUILD=""

for arg in "$@"; do
    if [[ "$arg" == "-gdb" ]]; then
        GDB_BUILD=true
    elif [[ -z "$TARGET" ]]; then
        TARGET="$arg"
    fi
done

case "$TARGET" in
    -a|--all|"")
        do_build ON ON ON "$GDB_BUILD"
        ;;
    -A|--all-but-gtest)
        do_build ON ON OFF "$GDB_BUILD"
        ;;
    -t|--custom-test)
        do_build ON OFF OFF "$GDB_BUILD"
        ;;
    -l|--longest)
        do_build OFF ON OFF "$GDB_BUILD"
        ;;
    -g|--gtest)
        do_build OFF OFF ON "$GDB_BUILD"
        ;;
    -c|--clean)
        clean
        ;;
    -cb|--clean-build)
        clean
        do_build ON ON ON "$GDB_BUILD"
        ;;
    -cc|--clean-cache)
        clean_cache
        ;;
    -clf|--clang-format)
        do_clang_format
        ;;
    -cmf|--cmake-format)
        do_cmake_format
        ;;
    -h|--help)
        help
        ;;
    *)
        echo "Unknown option: $TARGET"
        echo "Run '$0 -h' for help."
        exit 1
        ;;
esac
