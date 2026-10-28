# © Joseph Cameron - MIT License

set(_jfcsmarthandles_test_sources
    "${CMAKE_CURRENT_LIST_DIR}/allocation_failure_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/assignment_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/deleter_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/handle_types_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ownership_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shared_handle_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/unique_handle_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/weak_handle_test.cpp")

set(_jfcsmarthandles_test_libraries jfc-smart_handles)

if (NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)

    list(APPEND _jfcsmarthandles_test_sources "${CMAKE_CURRENT_LIST_DIR}/thread_test.cpp")
    list(APPEND _jfcsmarthandles_test_libraries Threads::Threads)
endif()

jfc_add_tests(
    C++_STANDARD 20

    TEST_SOURCE_FILES
        ${_jfcsmarthandles_test_sources}

    LIBRARIES
        ${_jfcsmarthandles_test_libraries}
)

if (NOT CMAKE_CROSSCOMPILING)
    add_test(NAME jfc-smart_handles_install_and_consume
        COMMAND "${CMAKE_COMMAND}"
            "-DBUILD_DIR=${PROJECT_BINARY_DIR}"
            "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/install_and_consume"
            "-DCONFIG=$<CONFIG>"
            "-DGENERATOR=${CMAKE_GENERATOR}"
            "-DPLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DTOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCXX_FLAGS=${CMAKE_CXX_FLAGS}"
            "-DEXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
            -P "${CMAKE_CURRENT_LIST_DIR}/install/install_and_consume.cmake")
endif()
