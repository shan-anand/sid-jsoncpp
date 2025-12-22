cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/sid)

# Test sources
set(TEST_SOURCES
    test_value.cpp
    test_parser.cpp
    test_format.cpp
    test_schema.cpp
    test_main.cpp
)

# Create test executable
add_executable(sid-json-tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(sid-json-tests 
    PRIVATE
    sid-json
    GTest::GTest
    GTest::Main
)

# Add coverage flags for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(sid-json-tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(sid-json-tests PRIVATE -fprofile-instr-generate)
    else()
        target_compile_options(sid-json-tests PRIVATE --coverage)
        target_link_options(sid-json-tests PRIVATE --coverage)
    endif()
endif()

# Add test
add_test(NAME sid-json-unit-tests COMMAND sid-json-tests)