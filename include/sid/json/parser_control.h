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

#define JSON_CPP_PARSE_MODE_ALLOW_FLEXIBLE_KEYS    1
#define JSON_CPP_PARSE_MODE_ALLOW_FLEXIBLE_STRINGS 2
#define JSON_CPP_PARSE_MODE_ALLOW_NOCASE_VALUES    4

namespace sid::json {

//! Parser control parameters
struct parser_control
{
  enum class dup_key : uint8_t {
    overwrite = 0, ignore, append, reject
  };

  union parse_mode
  {
    struct
    {
      uint8_t allowFlexibleKeys    : 1; //! If set to 1, accept key names not enclosed
                                        //!   within double-quotes
                                        //!   Must encode characters with unicode character
                                        //    (\u xxxx)
                                        //!     " -> \u
                                        //!     : -> \u
      uint8_t allowFlexibleStrings : 1; //! If set to 1, accept string values not enclosed
                                        //!   within double-quotes
                                        //!     " -> \u
                                        //!     , -> \u
                                        //!     ] -> \u
                                        //!     } -> \u
      uint8_t allowNocaseValues    : 1; //! If set to 1, it relaxes the parsing logic for
                                        //!   boolean and null types by accepting
                                        //!   True, TRUE, False, FALSE, Null, NULL
                                        //!   (in addition to true, false, null)
    };
    uint8_t flags;
    parse_mode(uint8_t _flags = 0) : flags(_flags) {}
  };

  //! Members
  parse_mode mode;    //! Parser control modes
  dup_key    dupKey;  //! Duplicate key handling

  //! Default constructor
  parser_control(
    const dup_key&    _dupKey = dup_key::overwrite,
    const parse_mode& _mode = parse_mode()
    ) : mode(_mode), dupKey(_dupKey)
    {}
};

} // namespace sid::json
