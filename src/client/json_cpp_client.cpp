#include "json_cpp/json.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

using namespace std;

namespace local
{
  std::string get_file_contents(const std::string& _filePath);
}

int main(int argc, char* argv[])
{
  int retVal = 0;
  try
  {
    if ( argc < 2 )
      throw std::invalid_argument("Need filename as an argument");
    /*
    cout << "sizeof(json::value) = " << sizeof(json::value) << endl;
    cout << "sizeof(json::schema) = " << sizeof(json::schema) << endl;
    cout << "sizeof(json::schema::property) = " << sizeof(json::schema::property) << endl;
    */
    std::string param, key, value;
    json::parser_control ctrl;
    std::optional<json::format> outputFmt;
    bool showOutput = false;
    for ( int i = 2; i < argc; i++ )
    {
      param = argv[i];
      if ( param[0] != '-' )
        throw std::invalid_argument("Invalid parameter at position " + std::to_string(i));
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
      if ( key == "--dup" || key == "--duplicate" )
      {
        if ( value == "accept" )
          ctrl.dupKey = json::parser_control::dup_key::accept;
        else if ( value == "ignore" )
          ctrl.dupKey = json::parser_control::dup_key::ignore;
        else if ( value == "append" )
          ctrl.dupKey = json::parser_control::dup_key::append;
        else if ( value == "reject" )
          ctrl.dupKey = json::parser_control::dup_key::reject;
        else if ( ! value.empty() )
          throw std::invalid_argument(key + " can only be accept|ignore|append|reject");
      }
      else if ( key == "--allow-flex-keys" || key == "--allow-flexible-keys" )
        ctrl.mode.allowFlexibleKeys = 1;
      else if ( key == "--allow-flex-strings" || key == "--allow-flexible-strings" )
        ctrl.mode.allowFlexibleStrings = 1;
      else if ( key == "--allow-nocase" || key == "--allow-nocase-values" )
        ctrl.mode.allowNocaseValues = 1;
      else if ( key == "--show-output" )
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
      else
        throw std::invalid_argument("Invalid key: " + key);
    }
    std::string jsonStr = local::get_file_contents(argv[1]);
    json::parser_stats stats;
    {
      json::value jroot;
      json::value::parse(jroot, stats, jsonStr, ctrl);
      if ( showOutput )
      {
        cout << (outputFmt.has_value()? jroot.to_str(outputFmt.value()) : jroot.to_str()) << endl;
      }
    }
    cout << stats.to_str() << endl;
    retVal = 0;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << endl;
    retVal = -1;
  }
  
  return retVal;
}

std::string local::get_file_contents(const std::string& _filePath)
{
  std::ifstream in;
  in.open(_filePath);
  if ( ! in.is_open() )
    throw std::system_error(errno, std::system_category(), "Failed to open file: " + _filePath);
  char buf[8096] = {0};
  std::string jsonStr;
  while ( ! in.eof() )
  {
    std::memset(buf, 0, sizeof(buf));
    in.read(buf, sizeof(buf)-1);
    if ( in.bad() )
    throw std::system_error(errno, std::system_category(), "Failed to read data");
    jsonStr += buf;
  }
  return jsonStr;
}
