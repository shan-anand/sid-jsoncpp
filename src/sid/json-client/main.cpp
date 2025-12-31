/*
LICENSE: BEGIN
===============================================================================
@author Shan Anand
@email anand.gs@gmail.com
@source https://github.com/shan-anand
@brief Json handling using c++
===============================================================================
MIT License

Copyright (c) 2017 Shanmuga (Anand) Gunasekaran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
LICENSE: END
*/

#include "json/json.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include <iterator>
#include <regex>

using namespace std;
using namespace sid;
using namespace sid::json;

namespace sid::json::local
{
  void show_usage(const char* _progName);
  std::string trim(const std::string& _str);
  string& get_stdin(string& _out);
  std::string& get_stream_contents(string& _out, std::ifstream& _in);
  std::string& get_file_contents(std::string& _out, const std::string& _filePath);
}

enum class Use { String, MMap, FileBuffer, StringBuffer, FileStream, StringStream };

int main(int argc, char* argv[])
{
  int retVal = 0;
  const bool isInteractiveMode = ::isatty(STDIN_FILENO) != 0;

  if ( isInteractiveMode && argc < 2 )
  {
    local::show_usage(argv[0]);
    return 1;
  }
  parser_output out;
  try
  {
    parser_control ctrl;
    std::string param, key, value;
    std::optional<json::format> outputFmt;
    bool isStdin = false;
    bool showOutput = false;
    std::optional<Use> use;
    std::optional<std::string> filename;
    // Parse for options and filename
    // They can be in any order
    // If filename is missing use stdin
    for ( int i = 1; i < argc; i++ )
    {
      param = argv[i];
      if ( param[0] != '-' )
      {
         // Any argument not starting with '-' is considered as filename
        // If filename is already set, then throw an error
        if ( filename.has_value() )
          throw std::invalid_argument("Filename already set");
        // This should be the filename (last argument)
        filename = param;
        if ( filename.value().empty() )
          throw std::invalid_argument("Filename cannot be empty");
        continue;
      }

      size_t pos = param.find('=');
      if ( pos == std::string::npos )
      {
        key = param;
        value.clear();
      }
      else
      {
        key = param.substr(0, pos);
        value = param.substr(pos+1);
      }
      if ( key == "--stdin")
      {
        isStdin = true;
      }
      else if ( key == "-h" || key == "--help" )
      {
        local::show_usage(argv[0]);
        return 1;
      }
      else if ( key == "-d" || key == "--dup" || key == "--duplicate" || key == "--duplicate-keys" )
      {
        if ( value == "overwrite" )
          ctrl.dupKey = json::parser_control::dup_key::overwrite;
        else if ( value == "ignore" )
          ctrl.dupKey = json::parser_control::dup_key::ignore;
        else if ( value == "append" )
          ctrl.dupKey = json::parser_control::dup_key::append;
        else if ( value == "reject" )
          ctrl.dupKey = json::parser_control::dup_key::reject;
        else if ( ! value.empty() )
          throw std::invalid_argument(key + " can only be overwrite|ignore|append|reject");
      }
      else if ( key == "-k" || key == "--allow-flex-keys" || key == "--allow-flexible-keys" )
        ctrl.mode.allowFlexibleKeys = 1;
      else if ( key == "-s" || key == "--allow-flex-strings" || key == "--allow-flexible-strings" )
        ctrl.mode.allowFlexibleStrings = 1;
      else if ( key == "-n" || key == "--allow-nocase" || key == "--allow-nocase-values" )
        ctrl.mode.allowNocaseValues = 1;
      else if ( key == "-o" || key == "--show-output" )
      {
        showOutput = true;
        if ( ! value.empty() )
        {
          if ( value == "false" || value == "no" )
            showOutput = false;
          else
            outputFmt = json::format::get(value);
        }
      }
      else if ( key == "-u" || key == "--use" )
      {
        if ( value == "mmap" )
          use = Use::MMap;
        else if ( value == "data" || value == "string" ) 
          use = Use::String;
        else if ( value == "file-buffer" )
          use = Use::FileBuffer;
        else if ( value == "string-buffer" )
          use = Use::StringBuffer;
        else if ( value == "file-stream" )
          use = Use::FileStream;
        else if ( value == "string-stream" )
          use = Use::StringStream;
        else
          throw std::invalid_argument(key + " values can only be mmap|string|file-buffer|string-buffer|file-stream|string-stream");
      }
      else
        throw std::invalid_argument("Invalid key: " + key);
    } // for loop

    if ( isInteractiveMode )
    {
      if ( isStdin && filename.has_value() )
        throw std::invalid_argument("Cannot use --stdin with filename");
      if ( !isStdin && !filename.has_value() )
        throw std::invalid_argument("Missing filename or --stdin");
    }
    else if ( isStdin || filename.has_value() )
     throw std::invalid_argument("Cannot use filename or --stdin with non-interactive mode");

    // --stdin or interactive modes cannot use mmap. if so, reset it
    if ( !filename.has_value() && use.has_value() && use.value() == Use::MMap )
      use.reset();
    // If use is not set, set the default value based on --stdin/interactive or <filename>
    if ( !use.has_value() )
      use = filename.has_value()? Use::MMap : Use::FileStream;

    if ( filename.has_value() )
    {
      switch ( use.value() )
      {
      case Use::MMap:
        {
          cerr << "Using mmap for parsing...." << endl;
          json::value::parse_file(out, filename.value(), ctrl);
        }
        break;
      case Use::String:
        {
          cerr << "Using string data for parsing...." << endl;
          std::string data; local::get_file_contents(data, filename.value());
          json::value::parse(out, data, ctrl);
        }
        break;
      case Use::FileBuffer:
        {
          cerr << "Using file buffer for parsing...." << endl;
          std::filebuf fbuf;
          //std::vector<char> buffer(512);
          //fbuf.pubsetbuf(buffer.data(), buffer.size());
          if ( !fbuf.open(filename.value().c_str(), std::ios::in) )
            throw std::system_error(errno, std::system_category(), "Failed to open file: " + filename.value());
          json::value::parse(out, fbuf, ctrl);
        }
        break;
      case Use::StringBuffer:
        {
          cerr << "Using string buffer for parsing...." << endl;
          std::string data; local::get_file_contents(data, filename.value());
          std::stringbuf sbuf(data, std::ios_base::in);
          json::value::parse(out, sbuf, ctrl);
        }
        break;
      case Use::FileStream:
        {
          cerr << "Using file stream for parsing...." << endl;
          std::ifstream fstream;
          fstream.open(filename.value());
          if ( !fstream.is_open() )
            throw std::system_error(errno, std::system_category(), "Failed to open file: " + filename.value());
          json::value::parse(out, fstream, ctrl);
        }
        break;
      case Use::StringStream:
        {
          cerr << "Using string stream for parsing...." << endl;
          std::string data; local::get_file_contents(data, filename.value());
          std::istringstream sstream(data);
          json::value::parse(out, sstream, ctrl);
        }
        break;
      }
    }
    else
    {
      // Check if stdin is from terminal (interactive) or pipe
      if ( isInteractiveMode )
      {
        // Interactive mode - show instruction
        cerr << "Reading multiple lines, end it with Ctrl+D" << endl;
      }
      switch ( use.value() )
      {
      case Use::MMap: // Cannot use MMap in stdin mode
        break;
      case Use::String:
        {
          cerr << "Using stdin string data for parsing...." << endl;
          std::string data; local::get_stdin(data);
          json::value::parse(out, data, ctrl);
        }
        break;
      case Use::FileBuffer:
        {
          cerr << "Using stdin file buffer for parsing...." << endl;
          // DISABLE THIS - Makes cin MUCH faster for reading character by character
          // This massively improves the performance
          std::ios_base::sync_with_stdio(false);
          // Also untie cin from cout (prevents flush on every read)
          std::cin.tie(nullptr);
          json::value::parse(out, *cin.rdbuf(), ctrl);
        }
        break;
      case Use::StringBuffer:
        {
          cerr << "Using stdin string buffer for parsing...." << endl;
          std::string data; local::get_stdin(data);
          std::stringbuf sbuf(data, std::ios_base::in);
          json::value::parse(out, sbuf, ctrl);
        }
        break;
      case Use::FileStream:
        {
          cerr << "Using stdin file stream for parsing...." << endl;
          // DISABLE THIS - Makes cin MUCH faster for reading character by character
          // This massively improves the performance
          std::ios_base::sync_with_stdio(false);
          // Also untie cin from cout (prevents flush on every read)
          std::cin.tie(nullptr);
          json::value::parse(out, cin, ctrl);
        }
        break;
      case Use::StringStream:
        {
          cerr << "Using stdin string stream for parsing...." << endl;
          std::string data; local::get_stdin(data);
          std::istringstream sstream(data);
          json::value::parse(out, sstream, ctrl);
        }
        break;
      }
    }
    if ( showOutput )
    {
      cout << (outputFmt.has_value()? out.jroot.to_string(outputFmt.value()) : out.jroot.to_string()) << endl;
    }
    cerr << out.stats.to_string() << endl;
    retVal = 0;
  }
  catch(const std::exception& e)
  {
    cerr << out.stats.to_string() << endl;
    cerr << "Error...: " << e.what() << endl;
    retVal = -1;
  }
  
  return retVal;
}

void local::show_usage(const char* _progName)
{
  const std::string usage =
R"~(Usage: ${PNAME} [options] [<json-file>|--stdin]
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
  ${PNAME} ./data.json               # Parse data.json file
  ${PNAME} --stdin                   # Read from stdin interactively
  ${PNAME} -o=pretty ./data.json     # Parse and show pretty output
  ${PNAME} -k -s ./data.json         # Allow flexible keys and strings
  ${PNAME} --dup=append ./data.json  # Append duplicate keys
  echo '{"key":"value"}' | ${PNAME}  # Parse from stdin (pipe)
  cat ./data.json | ${PNAME}         # Parse from stdin (pipe)
)~";

  // Replace all instances of "${PNAME}"" with _progName and print it
  std::regex pattern(R"(\$\{PNAME\})");  // Escape $ and {
  cerr << std::regex_replace(usage, pattern, _progName);
}

std::string local::trim(const std::string& _str)
{
  size_t first = _str.find_first_not_of(" \t\n\r");
  size_t last = _str.find_last_not_of(" \t\n\r");
  if (first == std::string::npos || last == std::string::npos)
    return "";
  return _str.substr(first, last - first + 1);
}

string& local::get_stdin(std::string& _out)
{
  _out.clear();
  // Get data from stdin. Reads all characters as it is.
  _out = std::string {
      std::istreambuf_iterator<char>(cin),
      std::istreambuf_iterator<char>()
  };
  // Get data from stdin, alternate method. Reads until newline of eof.
  //   If the last line doesn't end with a new line, it still adds a new line
  //for ( std::string line; getline(cin, line, '\0'); _out += line + "\n");
  return _out;
}

std::string& local::get_file_contents(std::string& _out, const std::string& _filePath)
{
  _out.clear();
  std::ifstream in;
  in.open(_filePath);
  if ( ! in.is_open() )
    throw std::system_error(errno, std::system_category(), "Failed to open file: " + _filePath);
  in.seekg(0, std::ios::end);
  size_t fileSize = in.tellg();
  in.seekg(0, std::ios::beg);
  if ( fileSize > 0 )
  {
    // Allocate enough space for the file contents
    _out.resize(fileSize);
    // Define the size to read
    const size_t BUFFER_SIZE = ::sysconf(_SC_PAGESIZE) * 32;
    char* p = _out.data();
    while ( ! in.eof() )
    {
      in.read(p, BUFFER_SIZE);
      if ( in.bad() )
        throw std::system_error(errno, std::system_category(), "Failed to read data");
      p += in.gcount();
    }
  }
  return _out;
}

/*
std::string local::get_file_contents(const std::string& _filePath)
{
  int fd = fd =::open(_filePath.c_str(), O_RDONLY);
  if ( fd < 0 )
    throw std::system_error(errno, std::system_category(), _filePath);

  try
  {
    // get the filesize statistics
    struct stat fileStat = {0};
    ::fstat(fd, &fileStat);
    std::string jsonStr;
    if ( fileStat.st_size > 0 )
    {
      // Set the iovec buffer size
      const size_t BUFFER_SIZE = ::sysconf(_SC_PAGESIZE) * 32;
      // Determine the number of iovec's needed
      const size_t iov_needed = (fileStat.st_size + BUFFER_SIZE - 1) / BUFFER_SIZE;
      const size_t iov_count = std::min(iov_needed, (size_t) IOV_MAX);
      // Allocate iovec on the stack
      struct iovec* iov = (struct iovec*) ::alloca(iov_count*sizeof(struct iovec));
      jsonStr.resize(fileStat.st_size);
      char* p = jsonStr.data();
      for ( size_t remaining = fileStat.st_size; remaining > 0; )
      {
        int i;
        for ( i = 0; i < iov_count && remaining > 0; i++ )
        {
          iov[i].iov_base = p;
          iov[i].iov_len = std::min(BUFFER_SIZE, remaining);
          p += BUFFER_SIZE;
          remaining -= iov[i].iov_len;
        }
        ssize_t r = ::readv(fd, iov, i);
        if ( r < 0 )
          throw std::system_error(errno, std::system_category(), "Failed to read data");
      }
    }
    ::close(fd);
    return jsonStr;
  }
  catch(const std::exception& e)
  {
    ::close(fd);
    throw;
  }
}

std::string local::get_file_contents(const std::string& _filePath)
{
  int fd = fd =::open(_filePath.c_str(), O_RDONLY);
  if ( fd < 0 )
    throw std::system_error(errno, std::system_category(), _filePath);

  try
  {
    // get the filesize statistics
    struct stat fileStat = {0};
    ::fstat(fd, &fileStat);
    std::string jsonStr;
    if ( fileStat.st_size > 0 )
    {
      // Set the iovec buffer size
      const size_t BUFFER_SIZE = ::sysconf(_SC_PAGESIZE) * 32;
      jsonStr.resize(fileStat.st_size);
      char* p = jsonStr.data();
      for ( size_t toRead, remaining = fileStat.st_size; remaining > 0; )
      {
        toRead = std::min(BUFFER_SIZE, remaining);
        ssize_t r = ::read(fd, p, toRead);
        if ( r < 0 )
          throw std::system_error(errno, std::system_category(), "Failed to read data");
        p += r;
        remaining -= r;
      }
      ::close(fd);
    }
    return jsonStr;
  }
  catch(const std::exception& e)
  {
    ::close(fd);
    throw;
  }
}
*/
