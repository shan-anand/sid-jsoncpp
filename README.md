# sid-json

A high-performance JSON parser and serializer library for C++17 and later.

## Overview

sid-json is a fast, lightweight JSON library that provides comprehensive JSON parsing, manipulation, and serialization capabilities. It supports flexible parsing modes, detailed statistics, and multiple output formats.

## Features

- **High Performance**: Optimized for speed with minimal memory overhead
- **C++17 Compatible**: Works with modern C++ standards
- **Flexible Parsing**: Support for relaxed JSON syntax including unquoted keys and values
- **Multiple Data Types**: Full support for all JSON types (null, boolean, numbers, strings, arrays, objects)
- **Detailed Statistics**: Built-in parsing statistics and timing information
- **Multiple Output Formats**: Compact and pretty-printed JSON output
- **Schema Validation**: Optional JSON schema validation support
- **Duplicate Key Handling**: Configurable handling of duplicate keys (accept, ignore, append, reject)
- **Comments Support**: Parse JSON with C++ and C-style comments

## Directory Structure

```
sid-json/
├── cmake/                  # CMake utilities
│   └── cmake_uninstall.cmake.in  # Uninstall script template
├── include/sid/json/       # Public headers
│   ├── json.h                 # Main include file
│   ├── value.h                # JSON value class
│   ├── parser_control.h       # Parser configuration
│   ├── format.h               # Output formatting
│   ├── parser_stats.h         # Parsing statistics
│   └── schema.h               # Schema validation (TODO)
├── src/sid/json/           # Implementation files
│   ├── value.cpp              # Implemenetaion of JSON value class
│   ├── parser.h               # Internal parser implementation
│   ├── parser_io.h            # Input structures for Character and Buffer parsers
│   ├── format.cpp             # Output formatting
│   ├── memory_map.h           # Memory mapping utilities
│   ├── parser_stats.cpp       # Implementation of parsing statistics
│   ├── schema.cpp             # Schema (TODO)
│   ├── time_calc.cpp          # Implementation of time utitilies
│   ├── time_calc.h            # Internal timing utilities
│   ├── utils.cpp              # Implementation of internal utility functions
│   └── utils.h                # Internal utility functions
├── src/sid/json-client/    # Client application
│   └── main.cpp               # Example/test client
├── tests/                  # Unit tests
│   ├── test_format.cpp        # Format tests
│   ├── test_main.cpp          # Test runner
│   ├── test_parser.cpp        # Parser tests
│   ├── test_schema.cpp        # Schema tests
│   ├── test_value.cpp         # Value class tests
│   ├── CMakeLists.txt         # Test build configuration
│   └── README.md              # Test documentation
├── CMakeLists.txt             # Build configuration
├── generate_json.py           # JSON test data generator
├── LICENSE                    # MIT License
└── README.md                  # This file
```

## Data Types

The library supports all standard JSON data types through the `json::value` class:

### Value Types
- `null` - JSON null value
- `boolean` - true/false values
- `_signed` - Signed 64-bit integers
- `_unsigned` - Unsigned 64-bit integers  
- `_double` - Long double precision floating point
- `string` - UTF-8 strings
- `array` - Ordered collections (std::vector<value>)
- `object` - Key-value maps (std::map<std::string, value>)

### Type Checking
```cpp
json::value val;
if (val.is_null()) { /* handle null */ }
if (val.is_string()) { /* handle string */ }
if (val.is_num()) { /* handle any numeric type */ }
if (val.is_array()) { /* handle array */ }
if (val.is_object()) { /* handle object */ }
```

## Building

### Using CMake
```bash
mkdir build
cd build
cmake ..
make
```

This will create a `libsid-json` library that can be linked to your projects.

### Testing
```bash
# Build with tests
cmake -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
make

# Run tests
ctest

# Run with verbose output
ctest --verbose

# Generate coverage report
# For GCC:
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# For Clang:
LLVM_PROFILE_FILE="coverage.profraw" ctest
llvm-profdata merge -sparse coverage.profraw -o coverage.profdata
llvm-cov show ./tests/sid-json-tests -instr-profile=coverage.profdata -format=html -output-dir=coverage_html
```

### Installation
```bash
# Install to system directories (requires sudo)
sudo make install

# Or install to custom location (no sudo needed)
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install
```

### Uninstallation
```bash
# Uninstall from system directories (requires sudo)
sudo make uninstall

# Or uninstall from custom location (no sudo needed)
make uninstall
```

### Integration
Add to your CMakeLists.txt:
```cmake
find_library(SID_JSON_LIB sid-json)
target_link_libraries(your_target ${SID_JSON_LIB})
```

## Usage Examples

### Basic Parsing
```cpp
#include <sid/json/json.h>

std::string json_str = R"({"name": "John", "age": 30, "active": true})";
json::value root;

if (json::value::parse(root, json_str)) {
    std::string name = root["name"].get_str();
    int age = root["age"].get_int64();
    bool active = root["active"].get_bool();
}
```

### Creating JSON
```cpp
json::value obj;
obj["name"] = "Alice";
obj["age"] = 25;
obj["scores"].append(95);
obj["scores"].append(87);
obj["scores"].append(92);

std::string json_output = obj.to_str(json::format_type::pretty);
```

### Parser Configuration
```cpp
json::parser_control ctrl;
ctrl.mode.allowFlexibleKeys = true;     // Allow unquoted keys
ctrl.mode.allowFlexibleStrings = true;  // Allow unquoted strings
ctrl.dupKey = json::parser_control::dup_key::append; // Append duplicate keys

json::value result;
json::value::parse(result, json_string, ctrl);
```

### Statistics
```cpp
json::value result;
json::parser_stats stats;

if (json::value::parse(result, stats, json_string)) {
    std::cout << "Objects: " << stats.objects << std::endl;
    std::cout << "Arrays: " << stats.arrays << std::endl;
    std::cout << "Parse time: " << stats.time_ms << "ms" << std::endl;
}
```

### Output Formatting
```cpp
json::format fmt(json::format_type::pretty);
fmt.indent = 4;
fmt.separator = ' ';
fmt.key_no_quotes = false;

std::string formatted = obj.to_str(fmt);
```

## Parser Features

### Flexible Parsing Modes
- **Flexible Keys**: Accept object keys without quotes
- **Flexible Strings**: Accept string values without quotes
- **Case-Insensitive Values**: Accept True/FALSE/NULL variants

### Duplicate Key Handling
- **Overwrite**: Overwrite with latest value (default)
- **Ignore**: Keep first value, ignore duplicates
- **Append**: Convert to array and append all values
- **Reject**: Throw error on duplicate keys

### Comments Support
- Shell/Python style comments (`# comment`)
- C++ style comments (`// comment`)
- C style comments (`/* comment */`)

## Performance

The library is optimized for performance with:
- Minimal memory allocations
- Efficient string handling
- Fast numeric parsing
- Built-in timing measurements

## Client (sid-json-client)

The sid-json library includes a command-line client application for parsing and validating JSON files.

```
Usage: sid-json-client [options] [<json-file>|--stdin]
       Interactive mode: Requires either <json-file> or --stdin
       Pipe mode: Automatically reads from stdin
       Tip: It's a good practice to start relative paths with ./
            Example: ./myfile.json  ./config/config.json
Options: <key>[=<value>]
  <key>
  -h, --help                     Show this help message
      --stdin                    Read from stdin (interactive mode only)
  -d, --dup, --duplicate         Duplicate key handling
      --duplicate-keys=<mode>      (mode: overwrite|ignore|append|reject)
                                   If omitted, it defaults to overwrite
  -k, --allow-flex-keys,         Allow unquoted object keys
      --allow-flexible-keys
  -s, --allow-flex-strings,      Allow unquoted string values
      --allow-flexible-strings
  -n, --allow-nocase,            Allow case-insensitive values for true, false, null
      --allow-nocase-values         * True, TRUE, False, FALSE, Null, NULL
  -o, --show-output[=<format>]   Show parsed JSON output
                                   (format: compact|pretty)
                                   If <format> is omitted, it defaults to compact
  -u, --use=<method>             Parsing method to use
                                   (method: mmap|string|file-buffer|string-buffer|file-stream|string-stream)
                                   If omitted, it defaults to
                                     * mmap for <filename>
                                     * string-stream for --stdin
                                   Note: mmap for --stdin is invalid and ignored
Examples:
  sid-json-client ./data.json               # Parse data.json file
  sid-json-client --stdin                   # Read from stdin interactively
  sid-json-client -o=pretty ./data.json     # Parse and show pretty output
  sid-json-client -k -s ./data.json         # Allow flexible keys and strings
  sid-json-client --dup=append ./data.json  # Append duplicate keys
  echo '{"key":"value"}' | sid-json-client  # Parse from stdin (pipe)
  cat ./data.json | sid-json-client         # Parse from stdin (pipe)
```

## License

MIT License - see LICENSE file for details.

## Author

Shan Anand (anand.gs@gmail.com)  
Source: https://github.com/shan-anand/sid-json