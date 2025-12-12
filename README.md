# sid-jsoncpp

A high-performance JSON parser and serializer library for C++17 and later.

## Overview

sid-jsoncpp is a fast, lightweight JSON library that provides comprehensive JSON parsing, manipulation, and serialization capabilities. It supports flexible parsing modes, detailed statistics, and multiple output formats.

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
sid-jsoncpp/
├── include/sid/jsoncpp/       # Public headers
│   ├── json.h                 # Main include file
│   ├── value.h                # JSON value class
│   ├── format.h               # Output formatting
│   ├── parser_control.h       # Parser configuration
│   ├── parser_stats.h         # Parsing statistics
│   └── schema.h               # Schema validation
├── src/sid/jsoncpp/           # Implementation files
│   ├── format.cpp
│   ├── parser.cpp
│   ├── parser_stats.cpp
│   ├── schema.cpp
│   ├── time_calc.cpp
│   ├── utils.cpp
│   ├── value.cpp
│   ├── parser.h               # Internal parser header
│   ├── time_calc.h            # Internal timing utilities
│   ├── utils.h                # Internal utility functions
│   └── memory_map.h           # Memory mapping utilities
├── src/sid/jsoncpp-client/    # Client application
│   └── main.cpp               # Example/test client
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

This will create a `libsid-jsoncpp` library that can be linked to your projects.

### Integration
Add to your CMakeLists.txt:
```cmake
find_library(SID_JSONCPP_LIB sid-jsoncpp)
target_link_libraries(your_target ${SID_JSONCPP_LIB})
```

## Usage Examples

### Basic Parsing
```cpp
#include <sid/jsoncpp/json.h>

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

## Client (sid-jsoncpp-client)

The sid-jsoncpp library includes a command-line client application for parsing and validating JSON files.

```
Usage: sid-jsoncpp-client [options] [<json-file>]
       If <json-file> is omitted, reads from stdin
       Tip: It's a good practice to start relative paths with ./
            Example: ./myfile.json  ./config/config.json
Options: <key>[=<value>]
  <key>
  -h, --help                     Show this help message
  -d, --dup, --duplicate=<mode>  Duplicate key handling (mode: accept|ignore|append|reject)
                                 If omitted, it defaults to accept
  -k, --allow-flex-keys,         Allow unquoted object keys
      --allow-flexible-keys
  -s, --allow-flex-strings,      Allow unquoted string values
      --allow-flexible-strings
  -n, --allow-nocase,            Allow case-insensitive true/false/null
      --allow-nocase-values
  -o, --show-output[=<format>]   Show parsed JSON output (format: compact|pretty)
                                 If <format> is omitted, it defaults to compact
  -u, --use=<method>             Parsing method (method: mmap|data|string)
                                 Valid only if <filename> is provided, skipped for stdin
                                 If omitted, it defaults to mmap

Examples:
  sid-jsoncpp-client ./data.json               # Parse data.json file
  sid-jsoncpp-client --output ./data.json      # Parse and show output
  sid-jsoncpp-client -o=pretty ./data.json     # Parse and show pretty output
  sid-jsoncpp-client -k -s ./data.json         # Allow flexible keys and strings
  sid-jsoncpp-client --dup=append ./data.json  # Append duplicate keys
  echo '{"key":"value"}' | sid-jsoncpp-client     # Parse from stdin
  cat ./data.json | sid-jsoncpp-client -o # Parse stdin and show output
```

## License

MIT License - see LICENSE file for details.

## Author

Shan Anand (anand.gs@gmail.com)  
Source: https://github.com/shan-anand/sid-jsoncpp