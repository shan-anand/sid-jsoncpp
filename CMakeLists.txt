cmake_minimum_required(VERSION 3.10)
project(sid-json VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Include directories
include_directories(include)
include_directories(include/sid)

# Source files
set(SOURCES
    src/sid/json/utils.cpp
    src/sid/json/format.cpp
    src/sid/json/parser.cpp
    src/sid/json/parser_stats.cpp
    src/sid/json/time_calc.cpp
    src/sid/json/value.cpp
    src/sid/json/schema.cpp
)

# Header files
set(HEADERS
    include/sid/json/format.h
    include/sid/json/json.h
    include/sid/json/parser_control.h
    include/sid/json/parser_stats.h
    include/sid/json/schema.h
    include/sid/json/value.h
)

# Create the library
add_library(sid-json ${SOURCES} ${HEADERS})

add_executable(sid-json-client
  src/sid/json-client/main.cpp
  $<TARGET_OBJECTS:sid-json>
)
# Set library properties
set_target_properties(sid-json PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install targets
install(TARGETS sid-json sid-json-client
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/sid
    DESTINATION include
)

# Show install messages
set(CMAKE_INSTALL_MESSAGE ALWAYS)

# Uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()