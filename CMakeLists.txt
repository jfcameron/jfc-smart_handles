# © Joseph Cameron - MIT License

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

if (TARGET jfc::smart_handles)
    return()
endif()

project(jfc-smart_handles
    VERSION 1.0
    DESCRIPTION "RAII wrappers for so-called handle types found in OpenAL, OpenGL, LibClang, patterned after the smart pointers of stl 2011"
    LANGUAGES CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")

include(GNUInstallDirs)

option(JFCSMARTHANDLES_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(JFCSMARTHANDLES_BUILD_DOCS "Build documentation" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/permissive->)
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
    endif()

    include(CheckCXXCompilerFlag)

    foreach (_strict_flag "-Werror=c++20-extensions" "-Werror=c++23-extensions")
        string(MAKE_C_IDENTIFIER "HAS${_strict_flag}" _strict_flag_symbol)

        check_cxx_compiler_flag("${_strict_flag}" ${_strict_flag_symbol})

        if (${_strict_flag_symbol})
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${_strict_flag}>)
        endif()
    endforeach()
endif()

add_library(jfc-smart_handles INTERFACE)

add_library(jfc::smart_handles ALIAS jfc-smart_handles)

set_target_properties(jfc-smart_handles PROPERTIES EXPORT_NAME smart_handles)

target_include_directories(jfc-smart_handles
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_compile_features(jfc-smart_handles INTERFACE cxx_std_20)

include(CMakePackageConfigHelpers)

set(_jfcsmarthandles_package_directory "${CMAKE_INSTALL_DATADIR}/cmake/jfc-smart_handles")

install(TARGETS jfc-smart_handles EXPORT jfc-smart_handles-targets)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT jfc-smart_handles-targets
    FILE jfc-smart_handles-targets.cmake
    NAMESPACE jfc::
    DESTINATION "${_jfcsmarthandles_package_directory}")

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/jfc-smart_handles-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT)

file(WRITE "${PROJECT_BINARY_DIR}/jfc-smart_handles-config.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/jfc-smart_handles-targets.cmake\")
")

install(FILES
    "${PROJECT_BINARY_DIR}/jfc-smart_handles-config.cmake"
    "${PROJECT_BINARY_DIR}/jfc-smart_handles-config-version.cmake"
    DESTINATION "${_jfcsmarthandles_package_directory}")

if (JFCSMARTHANDLES_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (JFCSMARTHANDLES_BUILD_DOCS)
    jfc_generate_documentation_doxygen(
        README_PATH      README.md
        OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs

        INCLUDE_DIRECTORIES
            include/
    )
endif()
