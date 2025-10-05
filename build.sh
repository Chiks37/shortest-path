#!/bin/bash

function build() {
    mkdir -p build
    cd build && cmake .. && cmake --build .
}

function clang-format() {
    clang-format -i "$(find ./search_algos/ -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp")"
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
    rm -rf build
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
