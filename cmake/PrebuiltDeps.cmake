# Prebuilt 3rdparty dependency helper.
#
# Builds a heavy 3rdparty library once into a config-specific cache directory
# (cache/deps/<name>/<config>/) and exposes its artifact path. The build runs at configure time via
# a standalone cmake invocation and ONLY when the artifact is missing -- so editing the top-level
# CMakeLists never triggers a rebuild, and debug/release versions are kept separately and built
# independently.
#
# Requires DEPS_CACHE_DIR and DEP_CONFIG_DIR to be set by the caller.
#
# Arguments: NAME (cache subdir), SRC (source tree to build), CMAKE_ARGS (extra
# -D flags), OUT_VAR (variable set in the caller's scope to the found library),
# and LIB_GLOB -- one or more recursive patterns matched against the build dir.
# Several patterns let config-specific names be matched (TBB ships libtbb.so in
# Release but libtbb_debug.so in Debug).

function(ensure_prebuilt_dep)
    cmake_parse_arguments(
        D
        ""
        "NAME;SRC;OUT_VAR"
        "LIB_GLOB;CMAKE_ARGS"
        ${ARGN})

    set(_bindir "${DEPS_CACHE_DIR}/${D_NAME}/${DEP_CONFIG_DIR}")
    set(_patterns "")
    foreach(_p ${D_LIB_GLOB})
        list(APPEND _patterns "${_bindir}/${_p}")
    endforeach()

    file(GLOB_RECURSE _found ${_patterns})
    if(NOT _found)
        message(STATUS "[deps] building ${D_NAME} (${CMAKE_BUILD_TYPE}) -> ${_bindir}")
        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -S "${D_SRC}" -B
                                "${_bindir}" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${D_CMAKE_ARGS}
                        RESULT_VARIABLE _cfg_result)
        if(NOT _cfg_result EQUAL 0)
            message(FATAL_ERROR "[deps] configure of ${D_NAME} failed")
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} --build "${_bindir}" --parallel
                        RESULT_VARIABLE _build_result)
        if(NOT _build_result EQUAL 0)
            message(FATAL_ERROR "[deps] build of ${D_NAME} failed")
        endif()

        file(GLOB_RECURSE _found ${_patterns})
    else()
        message(STATUS "[deps] using cached ${D_NAME} (${CMAKE_BUILD_TYPE})")
    endif()

    if(NOT _found)
        message(FATAL_ERROR "[deps] ${D_NAME}: '${D_LIB_GLOB}' not found under ${_bindir}")
    endif()

    list(GET _found 0 _lib)
    set(${D_OUT_VAR}
        "${_lib}"
        PARENT_SCOPE)
endfunction()
