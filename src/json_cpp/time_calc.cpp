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

#include "time_calc.h"
#include <cstring>

using namespace json;

time_calc::time_calc()
{
  clear();
}

void time_calc::clear()
{
  ::memset(&t_start, 0, sizeof(t_start));
  ::memset(&t_end, 0, sizeof(t_end));
}

void time_calc::start()
{
  t_start = p_capture();
}

void time_calc::stop()
{
  t_end = p_capture();
}

uint64_t time_calc::diff_secs() const
{
  return diff_millisecs() / 1000;
}

uint64_t time_calc::diff_millisecs() const
{
  return diff_microsecs() / 1000;
}

uint64_t time_calc::diff_microsecs() const
{
  if ( t_end.tv_sec > t_start.tv_sec ||
       (t_end.tv_sec == t_start.tv_sec && t_end.tv_nsec >= t_start.tv_nsec) )
  {
    uint64_t s = (t_end.tv_sec - t_start.tv_sec) * 1000000;
    uint64_t x = t_end.tv_nsec / 1000;
    uint64_t y = t_start.tv_nsec / 1000;
    if ( x < y )
    {
      --s;
      x += 1000000;
    }
    s += x;
    return s;
  }
  return 0;
}

struct timespec time_calc::p_capture() const
{
  struct timespec ts = {0};
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts;
}
