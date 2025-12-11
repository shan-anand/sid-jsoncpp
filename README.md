# json_cpp

A high-performance JSON parser and serializer library for C++11 and later.

## Overview

json_cpp is a fast, lightweight JSON library that provides comprehensive JSON parsing, manipulation, and serialization capabilities. It supports flexible parsing modes, detailed statistics, and multiple output formats.

## Features

- **High Performance**: Optimized for speed with minimal memory overhead
- **C++11 Compatible**: Works with modern C++ standards
- **Flexible Parsing**: Support for relaxed JSON syntax including unquoted keys and values
- **Multiple Data Types**: Full support for all JSON types (null, boolean, numbers, strings, arrays, objects)
- **Detailed Statistics**: Built-in parsing statistics and timing information
- **Multiple Output Formats**: Compact and pretty-printed JSON output
- **Schema Validation**: Optional JSON schema validation support
- **Duplicate Key Handling**: Configurable handling of duplicate keys (accept, ignore, append, reject)
- **Comments Support**: Parse JSON with C++ and C-style comments

## Directory Structure

```
json_cpp/
├── include/json_cpp/          # Public headers
│   ├── json.h                 # Main include file
│   ├── value.h                # JSON value class
│   ├── format.h               # Output formatting
│   ├── parser_control.h       # Parser configuration
│   ├── parser_stats.h         # Parsing statistics
│   ├── schema.h               # Schema validation
│   └── utils.h                # Utility functions
├── src/json_cpp/              # Implementation files
│   ├── format.cpp
│   ├── parser.cpp
│   ├── parser_stats.cpp
│   ├── time_calc.cpp
│   ├── utils.cpp
│   ├── value.cpp
│   ├── parser.h               # Internal parser header
│   └── time_calc.h            # Internal timing utilities
├── CMakeLists.txt             # Build configuration
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

This will create a `libjson_cpp` library that can be linked to your projects.

### Integration
Add to your CMakeLists.txt:
```cmake
find_library(JSON_CPP_LIB json_cpp)
target_link_libraries(your_target ${JSON_CPP_LIB})
```

## Usage Examples

### Basic Parsing
```cpp
#include <json_cpp/json.h>

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
- **Accept**: Overwrite with latest value
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

## License

MIT License - see LICENSE file for details.

## Author

Shan Anand (anand.gs@gmail.com)  
Source: https://github.com/shan-anand
