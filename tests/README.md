# sid-json Unit Tests

This directory contains comprehensive unit tests for the sid-json library using Google Test framework.

## Test Files

- `test_main.cpp` - Test runner and main entry point
- `test_value.cpp` - Tests for JSON value operations (types, conversions, containers)
- `test_parser.cpp` - Tests for JSON parsing (objects, arrays, errors, duplicate keys)
- `test_schema.cpp` - Tests for JSON schema validation and parsing

## Prerequisites

Install Google Test:
```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev

# macOS with Homebrew
brew install googletest

# Or build from source
git clone https://github.com/google/googletest.git
cd googletest && mkdir build && cd build
cmake .. && make && sudo make install
```

## Building and Running Tests

```bash
# From project root
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make

# Run all tests
ctest

# Run with verbose output
ctest --verbose

# Run specific test executable directly
./tests/sid-json-tests
```

## Code Coverage

```bash
# Build with coverage (Debug mode required)
cmake -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
make

# Run tests to generate coverage data
ctest

# Generate coverage report
# For GCC:
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# For Clang (if lcov fails):
LLVM_PROFILE_FILE="coverage.profraw" ctest
llvm-profdata merge -sparse coverage.profraw -o coverage.profdata
llvm-cov show ./tests/sid-json-tests -instr-profile=coverage.profdata -format=html -output-dir=coverage_html

# View coverage report
open coverage_html/index.html
```

## Test Coverage

### Value Tests
- Type checking and conversions
- Array and object operations
- Copy and assignment operations
- Default constructor behavior

### Parser Tests
- Basic JSON parsing (objects, arrays, primitives)
- Nested structure parsing
- Error handling for invalid JSON
- Duplicate key handling modes
- Comment parsing support
- Escaped string handling

### Schema Tests
- Schema type operations and validation
- Property constraints (min/max, length, items)
- Required field validation
- JSON schema parsing and conversion
- Schema object lifecycle management

### Format Tests
- Compact vs pretty formatting
- Custom indentation settings
- Nested structure formatting
- Special value formatting (null, boolean)
- String escaping in output
