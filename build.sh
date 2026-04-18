#!/bin/bash

BUILD_DIR="./build"
NETWORKIT_DIR="$BUILD_DIR/3rdparty/networkit"
TBB_DIR="$BUILD_DIR/3rdparty/oneTBB"
GDB_BUILD=""

function build() {
    if [[ "$2" == "-gdb" ]]; then
        GDB_BUILD=true
    fi
    mkdir -p build
    cd build \
    && cmake -DGDB_BUILD=$GDB_BUILD .. \
    && cmake --build .
}

function clang-format() {
    find ./app/ ./lib/ -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +
}

function cmake-format() {
    cmake-format -i "CMakeLists.txt"
}

function help() {
    echo "Usage: $0 <-c|-b|-cb|-clf|-cmf|-h>"
    echo "Keys:"
    echo "  -c, --clean          Remove build/ and bin/ directories"
    echo "  -b, --build          Build the project with [-gdb] optional flag for including debug data to bin"
    echo "  -cb, --clean-build   Execute -c and then -b"
    echo "  -clf, --clang-format Format C/C++ files using clang-format"
    echo "  -cmf, --cmake-format Format CMakeLists.txt using cmake-format"
    echo "  -h, --help           Print help"
    echo ""
    echo "Example:"
    echo "  $0 -b"
}

function clean() {
    if [ -d "$BUILD_DIR" ]; then
        find "$BUILD_DIR" -mindepth 1 \
            -not -path "$NETWORKIT_DIR/*" \
            -not -path "$NETWORKIT_DIR" \
            -not -path "$TBB_DIR/*" \
            -not -path "$TBB_DIR" \
            -not -path "${NETWORKIT_DIR%/*}" \
            -delete
    fi

    rm -rf bin
}

case $1 in
    -c|--clean)
        clean
        ;;
    -b|--build)
        build "$@"
        ;;
    -cb|--clean-build)
        clean
        build "$@"
        ;;
    -clf|--clang-format)
        clang-format
        ;;
    -cmf|--cmake-format)
        cmake-format
        ;;
    -h|--help)
        help
        ;;
    *)
        echo "Usage: $0 <-b|-f|-c>"
        exit 1
esac
