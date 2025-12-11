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
 * @file  parser_stats.cpp
 * @brief Implementation of json parser statistics
 */
#include "json_cpp/schema.h"
#include "utils.h"
#include "parser.h"
#include <sstream>
#include <iomanip>

using namespace json;
using namespace std;

//uint64_t json_gobjects_alloc = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of parser_stats
//
///////////////////////////////////////////////////////////////////////////////////////////////////
parser_stats::parser_stats()
{
  clear();
}

void parser_stats::clear()
{
  data_size = 0;
  objects = 0;
  arrays = 0;
  strings = 0;
  numbers = 0;
  booleans = 0;
  nulls = 0;
  keys = 0;
  time_ms = 0;
}

std::string parser_stats::to_str() const
{
  std::ostringstream out;
  out << "data size.....: " << json::get_sep(data_size) << " bytes" << endl
      << "objects.......: " << json::get_sep(objects) << endl
//      << " (" << json::get_sep(json_gobjects_alloc) << ")" << endl
      << "arrays........: " << json::get_sep(arrays) << endl
      << "strings.......: " << json::get_sep(strings) << endl
      << "numbers.......: " << json::get_sep(numbers) << endl
      << "booleans......: " << json::get_sep(booleans) << endl
      << "nulls.........: " << json::get_sep(nulls) << endl
      << "(keys)........: " << json::get_sep(keys) << endl
      << "(time taken)..: " << json::get_sep(time_ms/1000)
      << "." << std::setfill('0') << std::setw(3) << (time_ms % 1000) << " seconds" << endl
    ;
  return out.str();
}
