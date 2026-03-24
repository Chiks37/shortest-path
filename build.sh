#!/bin/bash

BUILD_DIR="./build"
KEEP_DIR="$BUILD_DIR/3rdparty/networkit"

function build() {
    mkdir -p build
    cd build && cmake .. && cmake --build .
}

function clang-format() {
    find ./search_algos/ -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +
}

function cmake-format() {
    cmake-format -i "CMakeLists.txt"
}

function help() {
    echo "Usage: $0 <-c|-b|-cb|-clf|-cmf|-h>"
    echo "Keys:"
    echo "  -c, --clean          Remove build/ and bin/ directories"
    echo "  -b, --build          Build the project"
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
            -not -path "$KEEP_DIR/*" \
            -not -path "$KEEP_DIR" \
            -delete
    fi

    rm -rf bin
}

case $1 in
    -c|--clean)
        clean
        ;;
    -b|--build)
        build
        ;;
    -cb|--clean-build)
        clean
        build
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
