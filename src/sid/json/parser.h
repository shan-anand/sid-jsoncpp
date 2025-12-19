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
#include <stack>
#include <string>
#include <cstdint>

namespace sid::json {

/**
 * @struct parser
 * @brief Internal json parser
 */
struct parser
{
  using ContinerStack = std::stack<value_type>;

  const parser_input& m_in;
  parser_output&      m_out;
  schema*             m_schema; //! Optional schema to validate against
  ContinerStack       m_containerStack; //! Container stack

  //! constructor
  parser(const parser_input& _in, parser_output& _out);

  //! parse and convert to json object
  bool parse();

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
  const char* m_eof;

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
  void parse_number(value& _jnum);
  //! parse json value
  void parse_value(value& _jval);

  //! check for space character
  bool is_space(const char* _p);
  //! are we at end of data
  bool is_eof(const char* _p) const;
  //! remove leading spaces and comments
  void skip_leading_spaces(const char*& _p);
};

} // namespace sid::json
