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

#pragma once

#include "json/value.h"
#include "json/schema.h"
#include "json/parser_stats.h"
#include "json/parser_control.h"
#include "parser_io.h"
#include "time_calc.h"
#include "utils.h"
#include "memory_map.h"
#include <istream>
#include <stack>
#include <string>
#include <cstdint>
#include <memory>
#include <cstring>

namespace sid::json {

/**
 * @struct parser
 * @brief Internal json parser
 */
template <typename Derived, typename parser_input, typename pos_type>
struct parser
{
  const parser_input& m_in;
  parser_output&      m_out;
  schema*             m_schema; //! Optional schema to validate against

  //! parse and convert to json object. Throws std::exception if parsing fails.
  void parse();

protected:
  using ContinerStack = std::stack<value_type>;
  ContinerStack       m_containerStack; //! Container stack

  //! constructor
  parser(const parser_input& _in, parser_output& _out)
    : m_in(_in), m_out(_out), m_schema(nullptr), m_containerStack() {}

private:
  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }

  //! derived classes callback
  // All the following functions must have a
  //   derived class implementation starting with s_
  ///////////////////////////////////////////////////////
  void init() { derived().s_init(); }
  pos_type tellg() const { return derived().s_tellg(); }
  pos_type seekg(pos_type _pos) { return derived().s_seekg(_pos); }
  char peek() { return derived().s_peek(); }
  char next() { return derived().s_next(); }
  bool eof() const { return derived().s_eof(); }
  size_t processed() const { return derived().s_processed(); }
  // This function should not change the current position
  pos_type add(pos_type _pos,  int _value) const { return derived().s_add(_pos, _value); }
  ///////////////////////////////////////////////////////

  struct line_info
  {
    pos_type begin; //! Beginning of word position
    uint64_t count; //! Current line number
  };
  //! Key value for object. It is reused in recursion.
  std::string m_key;
  line_info   m_line;

  inline void handle_newline() {
    ++m_line.count;
    m_line.begin = add(tellg(), 1);
  }

  //! get the current location of paring in the string
  inline std::string loc_str(const line_info& line, pos_type p) const {
    return std::string("@line:") + std::to_string(line.count) + ", @pos:" + std::to_string(p-line.begin+1);
  }
  inline std::string loc_str(const line_info& line) const { return loc_str(line, tellg()); }
  inline std::string loc_str(pos_type p) const { return loc_str(m_line, p); }
  inline std::string loc_str() const { return loc_str(m_line, tellg()); }

  //! parse object
  void parse_object(value& _jobj);
  //! parse array
  void parse_array(value& _jarr);
  //! parse key
  void parse_key(std::string& _str);
  //! parse string
  void parse_string(std::string& _str, bool _isKey);
  void parse_string(value& _jstr, bool _isKey);
  //! parser number
  void parse_number(value& _jnum);
  //! parse json value
  void parse_value(value& _jval);

  //! check for space character
  bool is_space();
  //! remove leading spaces and comments
  bool skip_leading_spaces();
};

struct char_parser : public parser<char_parser, char_parser_input, const char*>
{
  using pos_type = const char*;
  std::unique_ptr<memory_map> m_mmap;
  pos_type m_pos, m_first, m_last;

  //! constructor
  char_parser(const char_parser_input& _in, parser_output& _out)
    : parser(_in, _out) {}

  inline pos_type s_tellg() const { return m_pos; }
  inline pos_type s_seekg(pos_type _pos)
  {
    if ( _pos >= m_first && _pos <= m_last )
      m_pos = _pos;
    return m_pos;
  }
  inline char s_peek() const { return *m_pos; }
  inline char s_next() { if ( m_pos <= m_last ) ++m_pos; return *m_pos; }
  inline bool s_eof() const { return m_pos > m_last; }
  inline pos_type s_add(pos_type _pos,  int _value) const { return _pos + _value; }
  inline size_t s_processed() const { return static_cast<size_t>(m_pos-m_first); }

  void s_init()
  {
    switch ( m_in.inputType )
    {
    case input_type::data:
      // Set the first and last positions
      m_first = m_in.input.c_str();
      m_last = m_first + m_in.input.length() - 1;
      break;
    case input_type::file_path:
      m_mmap = std::make_unique<memory_map>(m_in.input);
      // Set the first and last positions
      m_first = m_mmap->begin();
      m_last = m_mmap->end();
      break;
    }
    s_seekg(m_first);
  }
};

struct buffer_parser : public parser<buffer_parser, buffer_parser_input, std::streampos>
{
  using pos_type = std::streampos;
  pos_type m_pos, m_first;
  //! constructor
  buffer_parser(const buffer_parser_input& _in, parser_output& _out)
    : parser(_in, _out) {}

  inline pos_type s_tellg() const { return m_pos; }
  inline pos_type s_seekg(pos_type _pos) {
    return (m_pos = m_in.sbuf.pubseekpos(_pos, std::ios_base::in));
  }
  inline char s_peek() const { return (char) m_in.sbuf.sgetc(); }
  inline char s_next() {
    int v = m_in.sbuf.snextc();
    if ( v != EOF ) m_pos = s_add(m_pos, 1);
    return (char) v;
  }
  inline bool s_eof() const { return m_in.sbuf.sgetc() == EOF; }
  inline pos_type s_add(pos_type _pos,  int _value) const {
    return _pos + static_cast<pos_type>(_value);
  }
  inline size_t s_processed() const { return static_cast<size_t>(m_pos-m_first+1); }

  void s_init()
  {
    m_first = m_pos = m_in.sbuf.pubseekoff(0, std::ios_base::beg, std::ios_base::in);
  }
};

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

//! parse and convert to json object
template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse()
{
  time_calc tc;

  try
  {
    if ( m_schema && m_schema->empty() )
      throw std::runtime_error("Invalid schema given for validation");

    m_out.clear();
    tc.start();
    // Call the specialization class's init method
    init();
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

    m_out.stats.data_size = processed();
    tc.stop();
    m_out.stats.time_ms = tc.diff_millisecs();
  }
  catch (...)
  {
    m_out.stats.data_size = processed();
    tc.stop();
    m_out.stats.time_ms = tc.diff_millisecs();
    throw;
  }
  //cout << "Object allocations: " << sid::get_sep(gobjects_alloc) << endl;
}

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_object(value& _jobj)
{
  char ch = 0;
  if ( ! _jobj.is_object() )
    _jobj.init(value_type::object);

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
        _jobj[m_key].init(value_type::array);
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

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_array(value& _jarr)
{
  char ch = 0;
  if ( ! _jarr.is_array() )
    _jarr.init(value_type::array);

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

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_key(std::string& _str)
{
  parse_string(_str, true);
}

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_string(std::string& _str, bool _isKey)
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

  bool finalGoNext = true; 
  for ( char ch = hasQuotes? next() : peek(); true; ch = next() )
  {
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
        { finalGoNext = false; break; }
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
  if ( finalGoNext )
    next();
}

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_string(value& _jstr, bool _isKey)
{
  std::string str;
  parse_string(str, _isKey);
  _jstr = str;
}

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_value(value& _jval)
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

template <typename Derived, typename parser_input, typename pos_type>
void parser<Derived, parser_input, pos_type>::parse_number(value& _jnum)
{
  skip_leading_spaces();
  pos_type start_pos = tellg();
  const char chContainer = (m_containerStack.top() == value_type::object)? '}' : ']' ;
  std::string numStr;
  number_info num;

  char ch = peek();
  if ( (num.integer.negative = (ch == '-')) )
  {
    numStr += ch;
    ch = next();
  }
  if ( ch < '0' || ch > '9' )
    throw std::runtime_error("Missing integer digit" + loc_str());
  numStr += ch;

  if ( ch == '0' )
  {
    ch = next();
    if ( ch >= '0' && ch <= '9' )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch) + ") after first 0 " + loc_str());
    num.integer.digits = 0;
    if ( !is_space() && ch != ',' && ch != chContainer )
      numStr += ch;
  }
  else
  {
    while ( (ch = next()) >= '0' && ch <= '9'  )
      numStr += ch;
  }
  // Check whether it has fraction and populate accordingly
  if ( ch == '.' )
  {
    numStr += ch;
    bool hasDigits = false;
    while ( (ch = next()) >= '0' && ch <= '9'  )
    {
      numStr += ch;
      hasDigits = true;
    }
    if ( !hasDigits )
      throw std::runtime_error("Invalid digit (" + std::string(1, ch)
                            + ") Expected a digit for fraction " + loc_str());
    num.hasFraction = true;
  }
  // Check whether it has an exponent and populate accordingly
  if ( ch == 'e' || ch == 'E' )
  {
    numStr += ch;
    ch = next();
    if ( ch == '-' || ch == '+' )
    {
      numStr += ch;
      ch = next();
    }
    bool hasDigits = false;
    for ( ; ch >= '0' && ch <= '9'; ch = next()  )
    {
      numStr += ch;
      hasDigits = true;
    }
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

  std::string errStr;
  //std::string numStr = get_str(start_pos, end_pos-start_pos), errStr;
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
template <typename Derived, typename parser_input, typename pos_type>
bool parser<Derived, parser_input, pos_type>::is_space()
{
  if ( peek() == '\n' )
  {
    handle_newline();
    return true;
  }
  return ::isspace(peek());
}

template <typename Derived, typename parser_input, typename pos_type>
bool parser<Derived, parser_input, pos_type>::skip_leading_spaces()
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

} // namespace sid::json
