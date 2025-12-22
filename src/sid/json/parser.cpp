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


//! parse and convert to json object
void parser::parse()
{
  auto start = [&]()
  {
    // Initialize local objects for parsing
    m_line.begin = tellg();
    m_line.count = 1;
    if ( !skip_leading_spaces() )
      throw std::runtime_error(std::string("End of data reached ") + loc_str() + ". Expecting { or [");

    switch ( peek() )
    {
    case '{':
      parse_object(m_out.jroot);
      break;
    case '[':
      parse_array(m_out.jroot);
      break;
    default:
      throw std::runtime_error(std::string("Invalid character [") + peek() + "] " + loc_str()
                          + ". Expecting { or [");
    }
    // Ensure there are no invalid trailing characters
    if ( skip_leading_spaces() )
      throw std::runtime_error(std::string("Invalid character [") + peek() + "] " + loc_str()
                        + " after the root " + to_str(m_out.jroot.type()) + " is closed");
  };

  time_calc tc;

  try
  {
    if ( m_schema && m_schema->empty() )
      throw std::runtime_error("Invalid schema given for validation");

    m_out.clear();
    tc.start();

    switch ( m_in.inputType )
    {
    case input_type::data:
      {
        // Set the size of the data
        m_out.stats.data_size = m_in.input.length();
        // Set the current position
        m_first = m_in.input.c_str();
        m_last = m_first + m_in.input.length() - 1;
        seekg(m_first);
        start();
      }
      break;
    case input_type::file_path:
      {
        memory_map mm(m_in.input);
        // Set the size of the data
        m_out.stats.data_size = mm.size();
        // Set the current position
        m_first = mm.begin();
        m_last = mm.end();
        seekg(m_first);
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
}

void parser::parse_object(value& _jobj)
{
  char ch = 0;
  if ( ! _jobj.is_object() )
    _jobj.p_set(value_type::object);

  m_containerStack.push(value_type::object);
  m_out.stats.objects++;
  for ( bool firstTime = true; true; firstTime = false )
  {
    next();
    // "string" : value
    if ( !skip_leading_spaces() )
      throw std::runtime_error("End of data reached " + loc_str() + " while expecting an object key or }");
    // This is the case where there are no elements in the object (An empty object)
    if ( peek() == '}' )
    {
      if ( !firstTime )
        throw std::runtime_error("End of object character } found at" + loc_str() + " while expecting a key");
      next();
      break;
    }

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
    if ( !skip_leading_spaces() )
      throw std::runtime_error("End of data reached " + loc_str() + " while expecting : for object key" + m_key);
    if ( peek() != ':' )
      throw std::runtime_error("Expected : " + loc_str() + " for object key" + m_key);
    next();
    if ( !skip_leading_spaces() )
      throw std::runtime_error("End of data reached " + loc_str() + " while expecting a value for object key" + m_key);
    // Handle new key
    if ( ! isDuplicateKey )
    {
      parse_value(_jobj[m_key]);
    }
    // Handle duplicate key based on the input mode
    else if ( m_in.ctrl.dupKey == parser_control::dup_key::overwrite )
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
    ch = peek();
    // Can have a ,
    // Must end with }
    if ( ch == '}' ) { next(); break; }
    if ( ch != ',' )
      throw std::runtime_error("Encountered " + std::string(1, peek()) + ". Expected , or } " + loc_str());
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
  for ( bool firstTime = true; true; firstTime = false)
  {
    next();
    // value
    if ( !skip_leading_spaces() )
      throw std::runtime_error("End of data reached " + loc_str() + " while expecting a value or ]");
    // This is the case where there are no elements in the array (An empty array)
    if ( peek() == ']' )
    {
      if ( !firstTime )
        throw std::runtime_error("End of array character ] found at" + loc_str() + " while expecting a value");
      next();
      break;
    }

    value& jval = _jarr.append();
    parse_value(jval);
    ch = peek();
    // Can have a ,
    // Must end with ]
    if ( ch == ']' ) { next(); break; }
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
    hasQuotes = (peek() == '\"');
  else if ( peek() != '\"' )
    throw std::runtime_error("Expected \" " + loc_str() + ", found \"" + std::string(1, peek()) + "\"");

  const line_info old_line = m_line;
  pos_type old_pos = tellg();

  auto check_hex = [&](char ch)->char
    {
      if ( eof() )
        throw std::runtime_error("Missing hexadecimal sequence characters at the end position "
                             + loc_str());
      if ( ! ::isxdigit(ch) )
        throw std::runtime_error("Missing hexadecimal character at " + loc_str());
      return ch;
    };

  char ch = 0;
  if ( !hasQuotes )
    prev();

  while ( true )
  {
    ch = next();
    if ( hasQuotes )
    {
      if ( eof() )
        throw std::runtime_error("Missing \" for string starting " + loc_str(old_line, old_pos));
      if ( ch == '\"' ) break;
      if ( ch == '\n' )
        handle_newline();
    }
    else
    {
      if ( eof() )
        throw std::runtime_error("End of string character not found for string starting " + loc_str());
      // Cannot have double-quotes, it must be escaped
      if ( ch == '\"' ) throw std::runtime_error("Character \" must be escaped " + loc_str());
      // A space character denotes end of the string
      if ( is_space() )
        break;
      // For a key : denotes end of the key
      // For a value , and the end of container key denotes end of the value
      if ( ( _isKey && ch == ':' ) || ( ! _isKey && (ch == ',' || ch == chContainer) ) )
        { prev(); break; }
    }
    if ( ch != '\\' ) { _str += ch; continue; }
    // We're encountered an escape character. Process it
    {
      ch = next();
      if ( eof() )
        throw std::runtime_error("Missing escape sequence characters at the end position " + loc_str());
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
          // Must be followed by 4 hex digits
          for ( int i = 0; i < 4; i++ )
            _str += check_hex(next());
        }
        break;
      default:
        throw std::runtime_error("Invalid escape sequence (" + std::string(1, ch) +
                             ") for string at " + loc_str());
      }
    }
  }
  next();
}

void parser::parse_string(value& _jstr, bool _isKey)
{
  std::string str;
  parse_string(str, _isKey);
  _jstr = str;
}

void parser::parse_value(value& _jval)
{
  if ( eof() )
    throw std::runtime_error("Unexpected end of data while expecting a value");

  char ch = peek();
  if ( ch == '{' )
    parse_object(_jval);
  else if ( ch == '[' )
    parse_array(_jval);
  else if ( ch == '\"' )
    parse_string(_jval, false);
  else if ( ch == '-' || ::isdigit(ch) )
    parse_number(_jval);
  else
  {
    const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;
    pos_type old_pos = tellg();
    const line_info old_line = m_line;

    char data[6] = {0}; // to capture null, true, false
    int len = 0;
    for ( char ch; !eof(); next(), len++ )
    {
      ch = peek();
      if ( len > 5 || ch == ',' || is_space() || ch == chContainer )
        break;
      data[len] = ch;
    }
    if ( tellg() == old_pos )
      throw std::runtime_error("Expected value not found " + loc_str());

    bool found = true;
    if ( len == 4 )
    {
      if ( ::strncmp(data, "null", len) == 0 )
        ;
      else if ( ::strncmp(data, "true", len) == 0 )
        _jval = true;
      else if ( ! m_in.ctrl.mode.allowNocaseValues )
        found = false;
      else
      {
        if ( ::strncmp(data, "Null", len) == 0 || ::strncmp(data, "NULL", len) == 0 )
          ;
        else if ( ::strncmp(data, "True", len) == 0 || ::strncmp(data, "TRUE", len) == 0 )
          _jval = true;
        else
          found = false;
      }
    }
    else if ( len == 5 )
    {
      if ( ::strncmp(data, "false", len) == 0 )
        _jval = false;
      else if ( ! m_in.ctrl.mode.allowNocaseValues )
        found = false;
      else
      {
        if ( ::strncmp(data, "False", len) == 0 || ::strncmp(data, "FALSE", len) == 0 )
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
        seekg(old_pos);
        m_line = old_line;
        parse_string(_jval, false);
      }
      else
        throw std::runtime_error("Invalid value [" + std::string(old_pos, len) + "] " + loc_str()
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

  skip_leading_spaces();
}

void parser::parse_number(value& _jnum)
{
  skip_leading_spaces();
  pos_type start_pos = tellg();
  const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;

  number_info num;

  if ( (num.integer.negative = (peek() == '-')) )
    next();
  char ch = peek();
  if ( ch < '0' || ch > '9' )
    throw std::runtime_error("Missing integer digit" + loc_str());

  if ( ch == '0' )
  {
    ch = next();
    if ( ch >= '0' && ch <= '9' )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch) + ") after first 0 " + loc_str());
    num.integer.digits = 0;
  }
  else
  {
    while ( (ch = next()) >= '0' && ch <= '9'  )
      ;
  }
  // Check whether it has fraction and populate accordingly
  if ( ch == '.' )
  {
    bool hasDigits = false;
    while ( (ch = next()) >= '0' && ch <= '9'  )
      hasDigits = true;
    if ( !hasDigits )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch)
                            + ") Expected a digit for fraction " + loc_str());
    num.hasFraction = true;
  }
  // Check whether it has an exponent and populate accordingly
  if ( ch == 'e' || ch == 'E' )
  {
    ch = next();
    if ( ch != '-' && ch != '+' )
      prev();
    bool hasDigits = false;
    while ( (ch = next()) >= '0' && ch <= '9'  )
      hasDigits = true;
    if ( !hasDigits )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch)
                            + ") Expected a digit for exponent " + loc_str());
    num.hasExponent = true;
  }

  const bool isNegative = num.integer.negative;
  const bool isDouble = ( num.hasFraction || num.hasExponent );

  pos_type end_pos = tellg();
  skip_leading_spaces();
  ch = peek();
  if ( !eof() && ch != ',' && ch != chContainer )
    throw std::runtime_error("Invalid character " + std::string(1, ch) + " Expected , or "
                         + std::string(1, chContainer) + " " + loc_str());

  std::string numStr = get_str(start_pos, end_pos-start_pos), errStr;
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

//! check for space character
bool parser::is_space()
{
  if ( peek() == '\n' )
  {
    handle_newline();
    return true;
  }
  return ::isspace(peek());
}

bool parser::skip_leading_spaces()
{
  do
  {
    for (; !eof() && is_space(); next() );
    if ( eof() ) return false;
    switch ( peek() )
    {
    case '#':
      // Shell/Python style comment encountered. Parse until end of line
      for ( next(); peek() != '\n' && !eof(); next() );
      break;
    case '/':
      next();
      if ( eof() )
        throw std::runtime_error("Invalid character at the end");
      switch ( peek() )
      {
      case '/':
        // C++ style comment encountered. Parse until end of line
        for ( next(); peek() != '\n' && !eof(); next() );
        break;
      case '*':
        {
          const line_info old_line = m_line;
          const pos_type old_pos = tellg();
          // C style comment encountered. Parse until */
          do
          {
            for ( next(); peek() != '*' && !eof(); next() )
            {
              if ( peek() == '\n' )
                handle_newline();
            }
            if ( eof() )
              throw std::runtime_error(std::string("Comments starting " + loc_str(old_line, old_pos))
                                    + " is not closed");
            next();
          }
          while ( peek() != '/' );
          next();
        }
        break;
      default:
        throw std::runtime_error(std::string("Invalid character [") + peek() + "] " + loc_str()
                              + " after the /");
      }
      break;
    default: // Any other character
      // That means we've skipped all the leading spaces and comments
      return true;
    }
  }
  while ( true );
}
