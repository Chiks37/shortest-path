# GAPBS integration: header-only INTERFACE target + our launcher compiled as a
# cached static library. Built into cache/deps/gapbs/ so it survives -c cleans.

# All GAPBS logic lives in headers (graph.h, builder.h, pvector.h, ...).
add_library(gapbs INTERFACE)
target_include_directories(gapbs INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/gapbs/src)

# gapbs_launcher wraps GAPBS types with our Launcher interface.
# OpenMP::OpenMP_CXX is already found by the top-level CMakeLists.
add_library(gapbs_launcher STATIC
    ${CMAKE_SOURCE_DIR}/lib/launcher/src/gapbs_launcher.cpp)
target_include_directories(gapbs_launcher PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/launcher/inc)
target_link_libraries(gapbs_launcher PRIVATE gapbs OpenMP::OpenMP_CXX)
