# © Joseph Cameron - All Rights Reserved

jfc_add_tests(
    C++_STANDARD 20

    TEST_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/shared_handle_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/unique_handle_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/weak_handle_test.cpp"

    LIBRARIES
            jfc-smart_handles
)

