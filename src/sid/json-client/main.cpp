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

using namespace std;
using namespace sid;
using namespace sid::json;

namespace sid::json::local
{
  void show_usage(const char* _progName);
  std::string trim(const std::string& _str);
  std::string get_file_contents(const std::string& _filePath);
}

int main(int argc, char* argv[])
{
  int retVal = 0;
  const bool isInteractiveMode = ::isatty(STDIN_FILENO) != 0;

  if ( isInteractiveMode && argc < 2 )
  {
    local::show_usage(argv[0]);
    return 1;
  }
  try
  {
    json::parser_input  in;
    std::string param, key, value;
    std::optional<json::format> outputFmt;
    bool isStdin = false;
    bool showOutput = false;
    bool useMmap = true;
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
      else if ( key == "-d" || key == "--dup" || key == "--duplicate" )
      {
        if ( value == "accept" )
          in.ctrl.dupKey = json::parser_control::dup_key::overwrite;
        else if ( value == "ignore" )
          in.ctrl.dupKey = json::parser_control::dup_key::ignore;
        else if ( value == "append" )
          in.ctrl.dupKey = json::parser_control::dup_key::append;
        else if ( value == "reject" )
          in.ctrl.dupKey = json::parser_control::dup_key::reject;
        else if ( ! value.empty() )
          throw std::invalid_argument(key + " can only be accept|ignore|append|reject");
      }
      else if ( key == "-k" || key == "--allow-flex-keys" || key == "--allow-flexible-keys" )
        in.ctrl.mode.allowFlexibleKeys = 1;
      else if ( key == "-s" || key == "--allow-flex-strings" || key == "--allow-flexible-strings" )
        in.ctrl.mode.allowFlexibleStrings = 1;
      else if ( key == "-n" || key == "--allow-nocase" || key == "--allow-nocase-values" )
        in.ctrl.mode.allowNocaseValues = 1;
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
          useMmap = true;
        else if ( value == "data" || value == "string" ) 
          useMmap = false;
        else
          throw std::invalid_argument(key + " values can only be mmap|data|string");
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

    std::string data;
    if ( filename.has_value() )
    {
      // Complete preparing the input object by filling the input type and input
      if ( useMmap )
      {
        cerr << "Using mmap for parsing...." << endl;
        in.set(json::input_type::file_path, filename.value());
      }
      else
      {
        data = local::get_file_contents(filename.value());
        cerr << "Using string data for parsing...." << endl;
        in.set(json::input_type::data, data);
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
      // Get data from stdin
      for ( std::string line; getline(cin, line); data += line);
      data = local::trim(data);
      if ( data.empty() ) return -1;
      in.set(json::input_type::data, data);
    }

    parser_output out;
    json::value::parse(in, out);
    if ( showOutput )
    {
      cout << (outputFmt.has_value()? out.jroot.to_str(outputFmt.value()) : out.jroot.to_str()) << endl;
    }
    cerr << out.stats.to_str() << endl;
    retVal = 0;
  }
  catch(const std::exception& e)
  {
    cerr << e.what() << endl;
    retVal = -1;
  }
  
  return retVal;
}

void local::show_usage(const char* _progName)
{
  cout << "Usage: " << _progName << " [options] [<json-file>|--stdin]" << endl
       << "       Interactive mode: Requires either <json-file> or --stdin" << endl
       << "       Pipe mode: Automatically reads from stdin" << endl
       << "       Tip: It's a good practice to start relative paths with ./" << endl
       << "            Example: ./myfile.json  ./config/config.json" << endl
       << "Options: <key>[=<value>]" << endl
       << "  <key>" << endl
       << "  -h, --help                     Show this help message" << endl
       << "      --stdin                    Read from stdin (interactive mode only)" << endl
       << "  -d, --dup, --duplicate=<mode>  Duplicate key handling (mode: accept|ignore|append|reject)" << endl
       << "                                 If omitted, it defaults to accept" << endl
       << "  -k, --allow-flex-keys,         Allow unquoted object keys" << endl
       << "      --allow-flexible-keys" << endl
       << "  -s, --allow-flex-strings,      Allow unquoted string values" << endl
       << "      --allow-flexible-strings" << endl
       << "  -n, --allow-nocase,            Allow case-insensitive true/false/null" << endl
       << "      --allow-nocase-values" << endl
       << "  -o, --show-output[=<format>]   Show parsed JSON output (format: compact|pretty)" << endl
       << "                                 If <format> is omitted, it defaults to compact" << endl
       << "  -u, --use=<method>             Parsing method (method: mmap|data|string)" << endl
       << "                                 Valid only if <filename> is provided, skipped for stdin" << endl
       << "                                 If omitted, it defaults to mmap" << endl
       << endl
       << "Examples:" << endl
       << "  " << _progName << " ./data.json               # Parse data.json file" << endl
       << "  " << _progName << " --stdin                   # Read from stdin interactively" << endl
       << "  " << _progName << " -o=pretty ./data.json     # Parse and show pretty output" << endl
       << "  " << _progName << " -k -s ./data.json         # Allow flexible keys and strings" << endl
       << "  " << _progName << " --dup=append ./data.json  # Append duplicate keys" << endl
       << "  echo '{\"key\":\"value\"}' | " << _progName << "  # Parse from stdin (pipe)" << endl
       << "  cat ./data.json | " << _progName << " -o      # Parse stdin (pipe) and show output" << endl;
}

std::string local::trim(const std::string& _str)
{
  size_t first = _str.find_first_not_of(" \t\n\r");
  size_t last = _str.find_last_not_of(" \t\n\r");
  if (first == std::string::npos || last == std::string::npos)
    return "";
  return _str.substr(first, last - first + 1);
}

std::string local::get_file_contents(const std::string& _filePath)
{
  std::ifstream in;
  in.open(_filePath);
  if ( ! in.is_open() )
    throw std::system_error(errno, std::system_category(), "Failed to open file: " + _filePath);
  in.seekg(0, std::ios::end);
  size_t fileSize = in.tellg();
  in.seekg(0, std::ios::beg);
  std::string jsonStr;
  if ( fileSize > 0 )
  {
    // Define the size to read
    const size_t BUFFER_SIZE = ::sysconf(_SC_PAGESIZE) * 32;
    // Allocate enough space for the file contents
    jsonStr.resize(fileSize);
    char* p = jsonStr.data();
    while ( ! in.eof() )
    {
      in.read(p, BUFFER_SIZE);
      if ( in.bad() )
        throw std::system_error(errno, std::system_category(), "Failed to read data");
      p += in.gcount();
    }
  }
  return jsonStr;
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
