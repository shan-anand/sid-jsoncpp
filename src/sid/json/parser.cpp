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
 * @file  parser.cpp
 * @brief Implementation of json parser and handler
 */
#include "parser.h"
#include "time_calc.h"
#include "utils.h"
#include "memory_map.h"
#include <fstream>
#include <stack>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <unistd.h>

using namespace sid::json;

//#define skip_leading_spaces(p)  for (; ::isspace(*p) && *p != '\0'; p++ );

//extern uint64_t json_gobjects_alloc;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of parser
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
From http://www.json.org/
___________________________________________________________________________________________________
|                              |                                   |                              |
|  object                      |  string                           |  number                      |
|      {}                      |      ""                           |      int                     |
|      { members }             |      "chars"                      |      int frac                |
|  members                     |  chars                            |      int exp                 |
|      pair                    |      char                         |      int frac exp            |
|      pair , members          |      char chars                   |  int                         |
|  pair                        |  char                             |      digit                   |
|      string : value          |      any-Unicode-character-       |      digit1-9 digits         |
|  array                       |          except-"-or-\-or-        |      - digit                 |
|      []                      |          control-character        |      - digit1-9 digits       |
|      [ elements ]            |      \"                           |  frac                        |
|  elements                    |      \\                           |      . digits                |
|      value                   |      \/                           |  exp                         |
|      value , elements        |      \b                           |      e digits                |
|  value                       |      \f                           |  digits                      |
|      string                  |      \n                           |      digit                   |
|      number                  |      \r                           |      digit digits            |
|      object                  |      \t                           |  e                           |
|      array                   |      \u four-hex-digits           |      e                       |
|      true                    |                                   |      e+                      |
|      false                   |                                   |      e-                      |
|      null                    |                                   |      E                       |
|                              |                                   |      E+                      |
|                              |                                   |      E-                      |
|                              |                                   |                              |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

//! Constructor
parser::parser(const parser_input& _in, parser_output& _out)
  : m_in(_in), m_out(_out), m_schema(nullptr)
{  
}

//! check for space character
bool parser::is_space(const char* _p)
{
  if ( *_p == '\n' )
  {
    ++m_line.count;
    m_line.begin = _p + 1;
    return true;
  }
  return ::isspace(*_p);
}

bool parser::is_eof(const char* _p) const
{
  return (_p > m_eof);
}

void parser::skip_leading_spaces(const char*& _p)
{
  do
  {
    for (; !is_eof(_p) && is_space(_p); _p++ );
    if ( is_eof(_p) || (*_p != '/' && *_p != '#') )
      return;

    if ( *_p == '#' )
    {
      // Shell/Python style comment encountered. Parse until end of line
      for ( _p++; *_p != '\n' && !is_eof(_p); _p++ );
    }
    else if ( *(_p+1) == '/' )
    {
      // C++ style comment encountered. Parse until end of line
      for ( _p++; *_p != '\n' && !is_eof(_p); _p++ );
    }
    else if ( *(_p+1) == '*' )
    {
      const line_info old_line = m_line;
      const char* old_p = _p;
      // C style comment encountered. Parse until */
      do
      {
        for ( _p++; *_p != '*' && !is_eof(_p); _p++ )
        {
          if ( *_p == '\n' )
          {
            ++m_line.count;
            m_line.begin = _p + 1;
          }
        }
        if ( *_p != '*' )
          throw std::runtime_error(std::string("Comments starting " + loc_str(old_line, old_p))
                               + " is not closed");
      }
      while ( *(_p+1) != '/' );
      _p += 2;
    }
  }
  while ( true );
}

//! parse and convert to json object
bool parser::parse()
{
  auto start = [&]()
  {
    m_line.begin = m_p;
    m_line.count = 1;
    skip_leading_spaces(m_p);
    if ( !is_eof(m_p) )
    {
      char ch = *m_p;
      if ( ch == '{' )
      {
        parse_object(m_out.jroot);
        skip_leading_spaces(m_p);
        if ( !is_eof(m_p) )
          throw std::runtime_error(std::string("Invalid character [") + *m_p + "] " + loc_str()
                            + " after the root object is closed");
      }
      else if ( ch == '[' )
      {
        parse_array(m_out.jroot);
        skip_leading_spaces(m_p);
        if ( !is_eof(m_p) )
          throw std::runtime_error(std::string("Invalid character [") + *m_p + "] " + loc_str()
                            + " after the root array is closed");
      }
      else
        throw std::runtime_error(std::string("Invalid character [") + *m_p + "] " + loc_str()
                            + ". Expecting { or [");
    }
    else
      throw std::runtime_error(std::string("End of data reached ") + loc_str() + ". Expecting { or [");
  };

  time_calc tc;

  try
  {
    if ( m_schema && m_schema->empty() )
      throw std::runtime_error("Invalid schema given for validation");

    tc.start();

    m_out.clear();

    switch ( m_in.inputType )
    {
    case input_type::data:
      {
        // Set the size of the data
        m_out.stats.data_size = m_in.input.length();
        // Set the current position
        m_p = m_in.input.c_str();
        m_eof = m_p + m_in.input.length() - 1;
        start();
      }
      break;
    case input_type::file_path:
      {
        memory_map mm(m_in.input);
        // Set the size of the data
        m_out.stats.data_size = mm.size();
        // Set the current position
        m_p = mm.begin();
        m_eof = mm.end();
        start();
      }
      break;
    }

    tc.stop();
    m_out.stats.time_ms = tc.diff_millisecs();
  }
  catch (...)
  {
    tc.stop();
    m_out.stats.time_ms = tc.diff_millisecs();
    throw;
  }

  //cout << "Object allocations: " << sid::get_sep(gobjects_alloc) << endl;
  // return the top-level json
  return true;
}

void parser::parse_object(value& _jobj)
{
  char ch = 0;
  if ( ! _jobj.is_object() )
    _jobj.p_set(value_type::object);

  m_containerStack.push(value_type::object);
  m_out.stats.objects++;
  while ( true )
  {
    ++m_p;
    // "string" : value
    skip_leading_spaces(m_p);
    // This is the case where there are no elements in the object (An empty object)
    if ( *m_p == '}' ) { ++m_p; break; }

    parse_key(m_key);
    // Check whether this key already exists in the object map
    bool isDuplicateKey = _jobj.has_key(m_key);
    if ( isDuplicateKey )
    {
      // Handle duplicate key scenario
      if ( m_in.ctrl.dupKey == parser_control::dup_key::reject )
        throw std::runtime_error("Duplicate key \"" + m_key + "\" encountered");
    }

    m_out.stats.keys++;
    skip_leading_spaces(m_p);
    if ( *m_p != ':' )
      throw std::runtime_error("Expected : " + loc_str());
    m_p++;
    skip_leading_spaces(m_p);
    if ( ! isDuplicateKey )
    {
      parse_value(_jobj[m_key]);
    }
    // Handle duplicate key based on the input mode
    else if ( m_in.ctrl.dupKey == parser_control::dup_key::accept )
    {
      // Accept the value and overwrite it
      parse_value(_jobj[m_key]);
    }
    else if ( m_in.ctrl.dupKey == parser_control::dup_key::ignore )
    {
      // Parse the value, but ignore it
      value jignore;
      parse_value(jignore);
    }
    else if ( m_in.ctrl.dupKey == parser_control::dup_key::append )
    {
      // make it as an array and append the duplicate keys
      if ( ! _jobj[m_key].is_array() )
      {
        // make a copy of the existing key's value
        value jcopy = _jobj[m_key];
        // make they key as an array
        _jobj[m_key].clear();
        _jobj[m_key].p_set(value_type::array);
        // append the copied value to the array
        _jobj[m_key].append(jcopy);
      }
      // Append the new value to the array
      value& jval = _jobj[m_key].append();
      parse_value(jval);
    }
    ch = *m_p;
    // Can have a ,
    // Must end with }
    if ( ch == '}' ) { ++m_p; break; }
    if ( ch != ',' )
      throw std::runtime_error("Encountered " + std::string(m_p, 1) + ". Expected , or } " + loc_str());
  }
  m_containerStack.pop();
}

void parser::parse_array(value& _jarr)
{
  char ch = 0;
  if ( ! _jarr.is_array() )
    _jarr.p_set(value_type::array);

  m_containerStack.push(value_type::array);
  m_out.stats.arrays++;
  while ( true )
  {
    ++m_p;
    // value
    skip_leading_spaces(m_p);
    // This is the case where there are no elements in the array (An empty array)
    if ( *m_p == ']' ) { ++m_p; break; }

    value& jval = _jarr.append();
    parse_value(jval);
    ch = *m_p;
    // Can have a ,
    // Must end with ]
    if ( ch == ']' ) { ++m_p; break; }
    if ( ch != ',' )
      throw std::runtime_error("Expected , or ] " + loc_str());
  }
  m_containerStack.pop();
}

void parser::parse_key(std::string& _str)
{
  parse_string(_str, true);
}

void parser::parse_string(std::string& _str, bool _isKey)
{
  _str.clear();
  const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;
  bool hasQuotes = true;
  if ( ( _isKey && m_in.ctrl.mode.allowFlexibleKeys ) || ( ! _isKey && m_in.ctrl.mode.allowFlexibleStrings ) )
    hasQuotes = (*m_p == '\"');
  else if ( *m_p != '\"' )
    throw std::runtime_error("Expected \" " + loc_str() + ", found \"" + std::string(1, *m_p) + "\"");

  const line_info old_line = m_line;
  const char* old_p = m_p;

  auto check_hex = [&](char ch)->char
    {
      if ( ch == '\0' )
        throw std::runtime_error("Missing hexadecimal sequence characters at the end position "
                             + loc_str());
      if ( ! ::isxdigit(ch) )
        throw std::runtime_error("Missing hexadecimal character at " + loc_str());
      return ch;
    };

  char ch = 0;
  if ( !hasQuotes )
    --m_p;

  while ( true )
  {
    ch = *(++m_p);
    if ( hasQuotes )
    {
      if ( ch == '\"' ) break;
      if ( ch == '\n' )
      {
        ++m_line.count;
        m_line.begin = m_p + 1;
      }
      else if ( ch == '\0' )
        throw std::runtime_error("Missing \" for string starting " + loc_str(old_line, old_p));
    }
    else
    {
      // Cannot have double-quotes, it must be escaped
      if ( ch == '\"' ) throw std::runtime_error("Character \" must be escaped " + loc_str());
      // A space character denotes end of the string
      if ( is_space(m_p) )
        break;
      // For a key : denotes end of the key
      // For a value , and the end of container key denotes end of the value
      if ( ( _isKey && ch == ':' ) || ( ! _isKey && (ch == ',' || ch == chContainer) ) )
        { --m_p; break; }
      if ( ch == '\0' )
        throw std::runtime_error("End of string character not found for string starting " + loc_str());
    }
    if ( ch != '\\' ) { _str += ch; continue; }
    // We're encountered an escape character. Process it
    {
      ch = *(++m_p);
      switch ( ch )
      {
      case '/':  _str += ch;   break;
      case 'b':  _str += '\b'; break;
      case 'f':  _str += '\f'; break;
      case 'n':  _str += '\n'; break;
      case 'r':  _str += '\r'; break;
      case 't':  _str += '\t'; break;
      case '\\': _str += ch;   break;
      case '\"': _str += ch;   break;
      case 'u':
      {
        const char* p = m_p;
        // Must be followed by 4 hex digits
        for ( int i = 0; i < 4; i++ )
          check_hex(*(++m_p));
        _str.append(p-1, 6);
      }
      break;
      case '\0':
        throw std::runtime_error("Missing escape sequence characters at the end position " + loc_str());
      default:
        throw std::runtime_error("Invalid escape sequence (" + std::string(1, ch) +
                             ") for string at " + loc_str());
      }
    }
  }
  ++m_p;
}

void parser::parse_string(value& _jstr, bool _isKey)
{
  std::string str;
  parse_string(str, _isKey);
  _jstr = str;
}

void parser::parse_value(value& _jval)
{
  char ch = *m_p;
  if ( ch == '{' )
    parse_object(_jval);
  else if ( ch == '[' )
    parse_array(_jval);
  else if ( ch == '\"' )
    parse_string(_jval, false);
  else if ( ch == '-' || ::isdigit(ch) )
    parse_number(_jval);
  else if ( ch == '\0' )
    throw std::runtime_error("Unexpected end of data while expecting a value");
  else
  {
    const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;
    const char* p_start = m_p;
    const line_info old_line = m_line;
    for ( char ch; (ch = *m_p) != '\0'; ++m_p )
    {
      if ( ch == ',' || is_space(m_p) || ch == chContainer )
        break;
    }
    if ( m_p == p_start )
      throw std::runtime_error("Expected value not found " + loc_str());

    bool found = true;
    size_t len = m_p - p_start;
    if ( len == 4 )
    {
      if ( ::strncmp(p_start, "null", len) == 0 )
        ;
      else if ( ::strncmp(p_start, "true", len) == 0 )
        _jval = true;
      else if ( ! m_in.ctrl.mode.allowNocaseValues )
        found = false;
      else
      {
        if ( ::strncmp(p_start, "Null", len) == 0 || ::strncmp(p_start, "NULL", len) == 0 )
          ;
        else if ( ::strncmp(p_start, "True", len) == 0 || ::strncmp(p_start, "TRUE", len) == 0 )
          _jval = true;
        else
          found = false;
      }
    }
    else if ( len == 5 )
    {
      if ( ::strncmp(p_start, "false", len) == 0 )
        _jval = false;
      else if ( ! m_in.ctrl.mode.allowNocaseValues )
        found = false;
      else
      {
        if ( ::strncmp(p_start, "False", len) == 0 || ::strncmp(p_start, "FALSE", len) == 0 )
          _jval = false;
        else
          found = false;
      }
    }
    else
      found = false;
    if ( ! found )
    {
      if ( m_in.ctrl.mode.allowFlexibleStrings )
      {
        m_p = p_start;
        m_line = old_line;
        parse_string(_jval, false);
      }
      else
        throw std::runtime_error("Invalid value [" + std::string(p_start, len) + "] " + loc_str()
                             + ". Did you miss enclosing in \"\"?");
    }
  }
  // Set the statistics of non-container objects here
  if ( _jval.is_string() )
    m_out.stats.strings++;
  else if ( _jval.is_num() )
    m_out.stats.numbers++;
  else if ( _jval.is_bool() )
    m_out.stats.booleans++;
  else if ( _jval.is_null() )
    m_out.stats.nulls++;

  skip_leading_spaces(m_p);
}

void parser::parse_number(value& _jnum)
{
  skip_leading_spaces(m_p);
  const char* p_start = m_p;
  const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;

  number_info num;
  // Perform full check and get the number
  if ( (num.integer.negative = (*m_p == '-')) )
    ++m_p;
  char ch = *m_p;
  if ( ch < '0' || ch > '9' )
    throw std::runtime_error("Missing integer digit" + loc_str());

  if ( ch == '0' )
  {
    ch = *(++m_p);
    if ( ch >= '0' && ch <= '9' )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch) + ") after first 0 " + loc_str());
    num.integer.digits = 0;
  }
  else
  {
    while ( (ch = *(++m_p)) >= '0' && ch <= '9'  )
      ;
  }
  // Check whether it has fraction and populate accordingly
  if ( ch == '.' )
  {
    bool hasDigits = false;
    while ( (ch = *(++m_p)) >= '0' && ch <= '9'  )
      hasDigits = true;
    if ( !hasDigits )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch)
                            + ") Expected a digit for fraction " + loc_str());
    num.hasFraction = true;
  }
  // Check whether it has an exponent and populate accordingly
  if ( ch == 'e' || ch == 'E' )
  {
    ch = *(++m_p);
    if ( ch != '-' && ch != '+' )
      --m_p;
    bool hasDigits = false;
    while ( (ch = *(++m_p)) >= '0' && ch <= '9'  )
      hasDigits = true;
    if ( !hasDigits )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch)
                            + ") Expected a digit for exponent " + loc_str());
    num.hasExponent = true;
  }

  const bool isNegative = num.integer.negative;
  const bool isDouble = ( num.hasFraction || num.hasExponent );
  const char* p_end = m_p;
  skip_leading_spaces(m_p);
  ch = *m_p;
  if ( ch != ',' && ch != '\0' && ch != chContainer )
    throw std::runtime_error("Invalid character " + std::string(1, ch) + " Expected , or "
                         + std::string(1, chContainer) + " " + loc_str());

  std::string numStr(p_start, p_end-p_start), errStr;
  if ( isDouble )
  {
    long double v = 0.0;
    if ( !json::to_num(numStr, /*out*/ v, &errStr) )
      throw std::runtime_error("Unable to convert (" + numStr + ") to double " + loc_str()
                           + ": " + errStr);
    _jnum = v;
  }
  else
  {
    if ( isNegative )
    {
      int64_t v = 0;
      if ( ! json::to_num(numStr, /*out*/ v, &errStr) )
        throw std::runtime_error("Unable to convert (" + numStr + ") to numeric " + loc_str()
                             + ": " + errStr);
      _jnum = v;
    }
    else
    {
      uint64_t v = 0;
      if ( ! json::to_num(numStr, /*out*/ v, &errStr) )
        throw std::runtime_error("Unable to convert (" + numStr + ") to numeric " + loc_str()
                             + ": " + errStr);
      _jnum = v;
    }
  }
}
