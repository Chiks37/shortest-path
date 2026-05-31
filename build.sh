#!/bin/bash

BUILD_DIR="./build"
DEPS_CACHE_DIR="./cache/deps"

function do_build() {
    local custom_test_networkit="$1"
    local custom_test_gapbs="$2"
    local longest="$3"
    local gtest="$4"
    local benchmark="$5"
    local gdb="$6"

    mkdir -p build
    cd build \
    && cmake \
        -DGDB_BUILD="$gdb" \
        -DBUILD_CUSTOM_TEST_NETWORKIT="$custom_test_networkit" \
        -DBUILD_CUSTOM_TEST_GAPBS="$custom_test_gapbs" \
        -DBUILD_LONGEST_PATH_SEARCH="$longest" \
        -DBUILD_GTEST="$gtest" \
        -DBUILD_BENCHMARK="$benchmark" \
        .. \
    && cmake --build .
}

function do_clang_format() {
    find ./app/ ./lib/ ./cmake/ -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +
}

function do_cmake_format() {
    cmake-format -i "CMakeLists.txt"
}

# Removes build/ and bin/ only; cache/deps/ (3rdparty artifacts) is preserved.
# Prebuilt deps live in cache/deps/<name>/<debug|release>/ and are reused as-is.
function clean() {
    rm -rf "$BUILD_DIR"
    rm -rf bin
}

# Removes the 3rdparty build cache (networkit, TBB, googletest, GAPBS) for both
# debug and release. Use when you need a truly fresh build of everything; the
# active configuration is rebuilt on the next build.
function clean_cache() {
    rm -rf "$DEPS_CACHE_DIR"
}

function help() {
    echo "Usage: $0 [TARGET] [-gdb]"
    echo "       $0 -c | -clf | -cmf | -h"
    echo ""
    echo "Build targets (default: -m):"
    echo "  -m,  --main                 Build custom_test_gapbs + longest + Google Tests (default)"
    echo "  -a,  --all                  Build ALL targets"
    echo "  -A,  --all-but-gtest        Build all executables, skip Google Tests"
    echo "  -tn, --custom-test-networkit  Build networkit-based custom_test_networkit only"
    echo "  -tg, --custom-test-gapbs    Build GAPBS-based custom_test_gapbs only"
    echo "  -l,  --longest              Build longest_path_search only"
    echo "  -g,  --gtest                Build and run Google Tests only"
    echo "  -b,  --benchmark            Build benchmark only"
    echo ""
    echo "Build modifiers:"
    echo "  -gdb                        Include full debug symbols (-O0 -g3 -ggdb)"
    echo ""
    echo "Other commands:"
    echo "  -c,  --clean                Remove build/ and bin/ (cache/deps/ is preserved)"
    echo "  -cb, --clean-build          Execute --clean then --all"
    echo "  -cc, --clean-cache          Remove cache/deps/ (3rdparty build artifacts)"
    echo "  -clf,--clang-format         Format C/C++ files using clang-format"
    echo "  -cmf,--cmake-format         Format CMakeLists.txt using cmake-format"
    echo "  -h,  --help                 Print this help"
    echo ""
    echo "Examples:"
    echo "  $0                    # build main set: custom_test_gapbs + longest + gtests"
    echo "  $0 -a                 # build everything"
    echo "  $0 -tg -gdb           # build custom_test_gapbs with debug symbols"
    echo "  $0 -g                 # build and run Google Tests"
    echo "  $0 -cb                # clean then build all (3rdparty cache preserved)"
    echo "  $0 -cc && $0 -a       # full rebuild including 3rdparty"
    echo ""
    echo "3rdparty deps (networkit, TBB, googletest) are cached per configuration"
    echo "in cache/deps/<name>/<debug|release>/ and are rebuilt ONLY when their"
    echo ".so/.a is missing. A -gdb (Debug) build never touches the Release cache"
    echo "and vice versa."
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

#                              nk    gapbs longest gtest bench
case "$TARGET" in
    -m|--main|"")
        do_build OFF ON  ON  ON  OFF "$GDB_BUILD"
        ;;
    -a|--all)
        do_build ON  ON  ON  ON  ON  "$GDB_BUILD"
        ;;
    -A|--all-but-gtest)
        do_build ON  ON  ON  OFF ON  "$GDB_BUILD"
        ;;
    -tn|--custom-test-networkit)
        do_build ON  OFF OFF OFF OFF "$GDB_BUILD"
        ;;
    -tg|--custom-test-gapbs)
        do_build OFF ON  OFF OFF OFF "$GDB_BUILD"
        ;;
    -l|--longest)
        do_build OFF OFF ON  OFF OFF "$GDB_BUILD"
        ;;
    -g|--gtest)
        do_build OFF OFF OFF ON  OFF "$GDB_BUILD"
        ;;
    -b|--benchmark)
        do_build OFF OFF OFF OFF ON  "$GDB_BUILD"
        ;;
    -c|--clean)
        clean
        ;;
    -cb|--clean-build)
        clean
        do_build ON  ON  ON  ON  ON  "$GDB_BUILD"
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
