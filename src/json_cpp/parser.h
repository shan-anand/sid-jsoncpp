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

#include "json_cpp/value.h"
#include "json_cpp/schema.h"
#include "json_cpp/parser_stats.h"
#include "json_cpp/parser_control.h"
#include <stack>
#include <string>
#include <cstdint>

namespace json {

/**
 * @struct parser
 * @brief Internal json parser
 */
struct parser
{
  value&         m_jroot;  //! value output object
  parser_stats&  m_stats;  //! statistics object
  parser_control m_ctrl;   //! Parser control flags
  std::string    m_input;  //! The Json string to be parsed
  schema*        m_schema; //! Optional schema to validate against
  std::stack<value_type> m_containerStack; //! Container stack

  //! constructor
  parser(value& _jout, parser_stats& _stats);

  //! parse the string and convert it to json object
  bool parse(const std::string& _value);

private:
  struct line_info
  {
    const char* begin; //! Beginning of word position
    uint64_t    count; //! Current line number
  };
  //! Key value for object. It is reused in recursion.
  std::string m_key;
  const char* m_p;
  line_info   m_line;

  //! get the current location of paring in the string
  inline std::string loc_str(const line_info& line, const char* p) const {
    return std::string("@line:") + std::to_string(line.count) + ", @pos:" + std::to_string(p-line.begin+1);
  }
  inline std::string loc_str(const line_info& line) const { return loc_str(line, m_p); }
  inline std::string loc_str(const char* p) const { return loc_str(m_line, p); }
  inline std::string loc_str() const { return loc_str(m_line, m_p); }

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
  void parse_number(value& _jnum, bool bFullCheck);
  //! parse json value
  void parse_value(value& _jval);

  //! check for space character
  bool is_space(const char* _p);
  //! remove leading spaces and comments
  void REMOVE_LEADING_SPACES(const char*& _p);
};

} // namespace json
