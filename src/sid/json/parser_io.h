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
