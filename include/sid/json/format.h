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

#include <string>
#include <cstdint>

namespace sid::json {

//! json format type
enum class format_type : uint8_t {
  compact, pretty
};

//! json format stucture
struct format
{
  format_type type;
  char        separator;
  uint32_t    indent;
  bool        key_no_quotes;
  bool        string_no_quotes;

  format() :
    type(format_type::compact), separator(' '),
    indent(2),
    key_no_quotes(false),
    string_no_quotes(false)
    {}
  format(
    const format_type& _type,
    bool               _key_no_quotes = false,
    bool               _string_no_quotes = false
    ) :
    format() {
      type =_type; key_no_quotes = _key_no_quotes; string_no_quotes = _string_no_quotes;
  }
  format(
    bool _key_no_quotes,
    bool _string_no_quotes
    ) :
    format() {
    key_no_quotes = _key_no_quotes; string_no_quotes = _string_no_quotes;
  }

  std::string to_string() const;
  static format get(const std::string& _value);
};

} // namespace sid::json
