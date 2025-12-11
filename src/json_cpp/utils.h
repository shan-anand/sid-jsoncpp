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
#include <vector>

#define SPLIT_TRIM          0x01
#define SPLIT_SKIP_EMPTY    0x02
#define SPLIT_TRIM_SKIP_EMPTY  (SPLIT_TRIM | SPLIT_SKIP_EMPTY)

namespace json {

std::string to_string(bool _value);
bool to_bool(const std::string& _str);
bool to_bool(const std::string& _str, bool& _out, std::string* _pstrError = nullptr);
bool to_num(const std::string& _str, long double& _out, std::string* _pstrError = nullptr);
bool to_num(const std::string& _str, uint32_t& _out, std::string* _pstrError = nullptr);
bool to_num(const std::string& _str, int64_t& _out, std::string* _pstrError = nullptr);
bool to_num(const std::string& _str, uint64_t& _out, std::string* _pstrError = nullptr);
std::string get_sep(uint64_t _number);
size_t split(
  std::vector<std::string>& _out,
  const std::string&        _str,
  const char                _delimiter,
  const uint32_t            _options = 0
);

} // namespace json
