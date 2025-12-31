/*
LICENSE: BEGIN
===============================================================================
@author Shan Anand
@email anand.gs@gmail.com
@source https://github.com/shan-anand
@file json.cpp
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

/**
 * @file  format.cpp
 * @brief Implementation of format structure
 */
#include "json/format.h"
#include "utils.h"
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <unistd.h>

using namespace sid::json;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of format
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/*static*/
format format::get(const std::string& _value)
{
  format fmt;

  std::vector<std::string> others;
  std::string typeStr = _value;
  size_t pos = _value.find(':');
  if ( pos != std::string::npos )
  {
    typeStr = _value.substr(0, pos);
    json::split(others, _value.substr(pos+1), ':', SPLIT_TRIM_SKIP_EMPTY);
  }
  if ( typeStr == "compact" )
    fmt.type = json::format_type::compact;
  else if ( typeStr == "xcompact" )
  {
    fmt.type = json::format_type::compact;
    fmt.key_no_quotes = true;
    //fmt.string_no_quotes = true;
  }
  else if ( typeStr == "pretty" )
    fmt.type = json::format_type::pretty;
  else if ( typeStr == "xpretty" )
  {
    fmt.type = json::format_type::pretty;
    fmt.key_no_quotes = true;
    //fmt.string_no_quotes = true;
  }
  else
    throw std::invalid_argument("Invalid format");

  std::string key, value, error;
  bool valueFound = false;
  for ( const std::string& other : others )
  {
    pos = other.find('=');
    key = other.substr(0, pos);
    valueFound = false;
    if ( pos != std::string::npos )
    {
      value = other.substr(pos+1);
      valueFound = true;
    }
    if ( key == "key-no-quotes" )
    {
      if ( ! valueFound )
        fmt.key_no_quotes = true;
      else if ( !json::to_bool(value, fmt.key_no_quotes, &error) )
        throw std::runtime_error("format " + key + " error: " + error);
    }
    else if ( key == "string-no-quotes" )
    {
      if ( ! valueFound )
        fmt.string_no_quotes = true;
      else if ( ! json::to_bool(value, fmt.string_no_quotes, &error) )
        throw std::runtime_error("Format " + key + " error: " + error);
    }
    else if ( key == "sep" || key == "separator" )
    {
      if ( fmt.type != json::format_type::pretty )
        throw std::runtime_error("Format separator is applicable only for pretty type" + error);
      if ( !valueFound || value.empty() || value == "s" || value == "space")
        value = " ";
      else if ( value == "t" || value == "tab" )
        value = "\t";
      if ( value.length() != 1 || ! ::isspace(value[0]) || value[0] == '\0' )
        throw std::runtime_error("Format separator must be a valid single space character");
      fmt.separator = value[0];
    }
    else if ( key == "indent" )
    {
      if ( fmt.type != json::format_type::pretty )
        throw std::runtime_error("Format indent is applicable only for pretty type");
      if ( ! valueFound )
        throw std::runtime_error("Format indent value is required");
      if ( ! json::to_num(value, fmt.indent, &error) )
        throw std::runtime_error("Format " + key + " error: " + error);
    }
    else
      throw std::runtime_error("Invalid format parameter: " + key);
  }
  return fmt;
}

std::string format::to_string() const
{
  std::ostringstream out;
  out << ((this->type == format_type::compact)? "compact":"pretty");
  if ( this->type == format_type::pretty )
  {
    out << ":sep=" << this->separator;
    out << ":indent=" << this->indent;
  }
  if ( this->key_no_quotes )
    out << ":key_no_quotes=" << json::to_string(this->key_no_quotes);
  if ( this->string_no_quotes )
    out << ":string_no_quotes=" << json::to_string(this->string_no_quotes);
  return out.str();
}
