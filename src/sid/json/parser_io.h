/*
LICENSE: BEGIN
===============================================================================
@author Shan Anand
@email anand.gs@gmail.com
@source https://github.com/shan-anand
@file parser_io.h
@brief Input structures for Character and Buffer parsers
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
 * @file  parser_io.h
 * @brief Input structures for Character and Buffer parsers
 */
#pragma once

#include <json/value.h>
#include <sstream>

namespace sid::json {

struct base_parser_input
{
  parser_control ctrl;

protected:
  //! Constructor
  base_parser_input(const parser_control& _ctrl = parser_control())
    : ctrl(_ctrl) {}
};

struct char_parser_input : public base_parser_input
{
  const std::string& input;
  input_type         inputType;

  //! Constructor
  char_parser_input(
    const std::string&    _input,
    const input_type      _inputType,
    const parser_control& _ctrl = parser_control())
    : base_parser_input(_ctrl), input(_input), inputType(_inputType) {}
};

struct buffer_parser_input : public base_parser_input
{
  std::streambuf& sbuf;

  //! Constructor
  buffer_parser_input(std::streambuf& _in,
                      const parser_control& _ctrl = parser_control())
    : base_parser_input(_ctrl), sbuf(_in) {}
};

} // namespace sid::json
