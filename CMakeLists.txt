cmake_minimum_required(VERSION 3.10)
project(json_cpp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/json_cpp/format.cpp
    src/json_cpp/parser.cpp
    src/json_cpp/parser_stats.cpp
    src/json_cpp/time_calc.cpp
    src/json_cpp/utils.cpp
    src/json_cpp/value.cpp
    src/json_cpp/schema.cpp
)

# Header files
set(HEADERS
    include/json_cpp/format.h
    include/json_cpp/json.h
    include/json_cpp/parser_control.h
    include/json_cpp/parser_stats.h
    include/json_cpp/schema.h
    include/json_cpp/utils.h
    include/json_cpp/value.h
)

# Create the library
add_library(json_cpp ${SOURCES} ${HEADERS})

add_executable(json_cpp_client
  src/client/json_cpp_client.cpp
  $<TARGET_OBJECTS:json_cpp>
)
# Set library properties
set_target_properties(json_cpp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install targets
install(TARGETS json_cpp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/json_cpp
    DESTINATION include
)