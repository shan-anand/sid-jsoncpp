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
 * @file  json.cpp
 * @brief Implementation of json parser and handler
 */
#include "json/value.h"
#include "json/schema.h"
#include "utils.h"
#include "parser.h"
#include <fstream>
#include <stack>
#include <iomanip>
#include <ctime>
#include <unistd.h>

using namespace std;
using namespace sid;
using namespace sid::json;

//extern uint64_t json_gobjects_alloc;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of value
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @fn bool parse(const parser_input& _in,
 *                parser_output&      _out
 *               );
 * @brief Convert the given json string to json object
 *
 * @param _in [in] parser input
 * @param _out [out] parser ouput
 */
/*static*/
void value::parse(
  const parser_input& _in,
  parser_output&      _out
  )
{
  parser jparser(_in, _out);
  //jparser.m_schema = &_in.schema;
  jparser.parse();

}

void value::p_set(const value_type _type/* = value_type::null*/)
{
  m_type = m_data.init(_type);
}

value::value(const value_type _type/* = value_type::null*/)
{
  m_type = m_data.init(_type);
}

value::value(const value& _obj)
{
  m_type = m_data.init(_obj.m_data, true, _obj.m_type);
}

// Move constructor
value::value(value&& _obj) noexcept
{
  m_type = m_data.init(std::move(_obj.m_data), _obj.m_type);
  _obj.m_type = value_type::null;
}

value::value(const int64_t _val)
{
  m_type = m_data.init(_val);
}

value::value(const uint64_t _val)
{
  m_type = m_data.init(_val);
}

value::value(const double _val)
{
  m_type = m_data.init(static_cast<long double>(_val));
}

value::value(const long double _val)
{
  m_type = m_data.init(_val);
}

value::value(const bool _val)
{
  m_type = m_data.init(_val);
}

value::value(const std::string& _val)
{
  m_type = m_data.init(_val);
}

value::value(const char* _val)
{
  m_type = m_data.init(_val);
}

value::value(const int _val)
{
  m_type = m_data.init(_val);
}

value::~value()
{
  clear();
}

void value::clear()
{
  m_type = m_data.clear(m_type);
}

value& value::operator=(const value& _obj)
{
  const bool is_new = ( m_type != _obj.m_type || m_type != value_type::object );
  if ( is_new )
    this->clear();
  m_type = m_data.init(_obj.m_data, is_new, _obj.m_type);
  return *this;
}

value& value::operator=(const int64_t _val)
{
  this->clear();
  m_type = m_data.init(_val);
  return *this;
}

value& value::operator=(const uint64_t _val)
{
  this->clear();
  m_type = m_data.init(_val);
  return *this;
}

value& value::operator=(const double _val)
{
  return operator=(static_cast<long double>(_val));
}

value& value::operator=(const long double _val)
{
  this->clear();
  m_type = m_data.init(_val);
  return *this;
}

value& value::operator=(const bool _val)
{
  this->clear();
  m_type = m_data.init(_val);
  return *this;
}

value& value::operator=(const std::string& _val)
{
  this->clear();
  m_type = m_data.init(_val);
  return *this;
}

value& value::operator=(const char* _val)
{
  this->clear();
  if ( _val != nullptr )
    m_type = m_data.init(std::string(_val));
  else
    m_type = m_data.init(value_type::null);
  return *this;
}

value& value::operator=(const int _val)
{
  return operator=(static_cast<int64_t>(_val));
}

bool value::has_index(const size_t _index) const
{
  if ( ! is_array() )
    throw std::runtime_error(__func__ + std::string("() can be used only for array type"));
  return ( _index < m_data._arr.size() );
}

bool value::has_key(const std::string& _key) const
{
  value jval;
  return has_key(_key, jval);
}

bool value::has_key(const std::string& _key, value& _obj) const
{
  if ( ! is_object() )
    throw std::runtime_error(__func__ + std::string("() can be used only for object type. ") + _key);
  if ( auto it = m_data.map().find(_key); it != m_data.map().end() )
  {
    _obj = it->second;
    return true;
  }
  return false;
}

std::vector<std::string> value::get_keys() const
{
  if ( ! is_object() )
    throw std::runtime_error(__func__ + std::string("() can be used only for object type"));
  std::vector<std::string> keys;
  for ( const auto& entry : m_data.map() )
    keys.push_back(entry.first);
  return keys;
}

size_t value::size() const
{
  if ( is_array() )
    return m_data._arr.size();
  else if ( is_object() )
    return m_data.map().size();
  throw std::runtime_error(__func__ + std::string("() can be used only for array and object types"));
}

const value::object_t& value::get_object() const
{
  if ( is_object() )
  {
    if ( !m_data._map )
      throw std::runtime_error(__func__ + std::string("() object is null"));
    return *m_data._map;
  }
  throw std::runtime_error(__func__ + std::string("() can be used only for object type"));
}

const value::array_t& value::get_array() const
{
  if ( is_array() )
    return m_data._arr;
  throw std::runtime_error(__func__ + std::string("() can be used only for array type"));
}

int64_t value::get_int64() const
{
  if ( is_num() )
    return m_data._i64;
  throw std::runtime_error(__func__ + std::string("() can be used only for number type"));
}

uint64_t value::get_uint64() const
{
  if ( is_num() )
    return m_data._u64;
  throw std::runtime_error(__func__ + std::string("() can be used only for number type"));
}

long double value::get_double() const
{
  if ( is_num() )
    return m_data._dbl;
  throw std::runtime_error(__func__ + std::string("() can be used only for number type"));
}

bool value::get_bool() const
{
  if ( is_bool() )
    return m_data._bval;
  throw std::runtime_error(__func__ + std::string("() can be used only for boolean type"));
}

std::string value::get_str() const
{
  if ( is_string() )
    return m_data._str;
  throw std::runtime_error(__func__ + std::string("() can be used only for string type"));
}

std::string value::as_str() const
{
  if ( is_string() )
    return m_data._str;
  else if ( is_bool() )
    return json::to_string(m_data._bval);
  else if ( is_signed() )
    return std::to_string(m_data._i64);
  else if ( is_unsigned() )
    return std::to_string(m_data._u64);
  else if ( is_double() )
    return std::to_string(m_data._dbl);
  throw std::runtime_error(
    __func__ + std::string("() can be used only for string, number or boolean types"));
}

int value::get_value(bool& _val) const
{
  if ( is_null() )
    return -1;
  _val = get_bool();
  return 1;
}

int value::get_value(std::string& _val) const
{
  if ( is_null() )
    return -1;
  _val = as_str();
  return 1;
}

//! Convert json to string using the given format type
std::string value::to_str(const format_type _type/* = format_type::compact*/) const
{
  std::ostringstream out;
  this->write(out, _type);
  return out.str();
}

//! Convert json to string using the given format
std::string value::to_str(const format& _format) const
{
  std::ostringstream out;
  this->write(out, _format);
  return out.str();
}

//! Write json to the given output stream
void value::write(std::ostream& _out, const format_type _type/* = format_type::compact*/) const
{
  if ( ! is_object() && ! is_array() )
    throw std::runtime_error("Can be applied only on a object or array");

  this->p_write(_out, format(_type), 0);
}

//! Write json to the given output stream using pretty format
void value::write(std::ostream& _out, const format& _format) const
{
  if ( ! is_object() && ! is_array() )
    throw std::runtime_error("Can be applied only on a object or array");

  if ( ! ::isspace(_format.separator) && _format.separator != '\0' )
    throw std::runtime_error("Format separator must be a valid space character. It cannot be \""
                         + std::string(1, _format.separator) + "\"");

  this->p_write(_out, _format, 0);
}

void value::p_write(std::ostream& _out, const format& _format, uint32_t _level) const
{
  auto escape_string =[&](const std::string& _input)->std::string
    {
      std::string output;
      for ( size_t i = 0; i < _input.length(); i++ )
      {
        char ch = _input[i];
        switch ( ch )
        {
          //case '/':
          //output += '\\';
          //output += ch;
          //break;
        case '\b':
          output += "\\b";
          break;
        case '\f':
          output += "\\f";
          break;
        case '\n':
          output += "\\n";
          break;
        case '\r':
          output += "\\r";
          break;
        case '\t':
          output += "\\t";
          break;
        case '\\':
          if ( _input[i+1] != 'u' || _format.string_no_quotes )
            output += '\\';
          output += ch;
          break;
        case '\"': // Quotation mark
          output += '\\';
          output += ch;
          break;
          /*
            case '\u':
            output += "\\u";
            break;
          */
        default:
          if ( ch == ',' && _format.string_no_quotes )
            output += "\\u002c";
          else
            output += ch;
          break;
        }
      }
      return output;
    };

  std::string padding;
  const char* final_padding = padding.c_str();;
  if ( _format.type == format_type::pretty && _format.separator != '\0' )
  {
    padding = std::string((_level+1) * _format.indent, _format.separator);
    final_padding = (padding.c_str() + _format.indent);
  }

  if ( is_object() )
  {
    _out << "{";
    bool isFirst = true;
    for ( const auto& entry : m_data.map() )
    {
      if ( ! isFirst )
      {
        _out << ",";
        if ( _format.type == format_type::pretty )
          _out << endl;
      }
      else if ( _format.type == format_type::pretty )
        _out << endl;
      isFirst = false;
      if ( _format.type == format_type::compact )
      {
        if ( ! _format.key_no_quotes )
          _out << "\"" << entry.first << "\":";
        else
          _out << entry.first << ":";
        entry.second.p_write(_out, _format, _level+1);
      }
      else if ( _format.type == format_type::pretty )
      {
        if ( ! _format.key_no_quotes )
          _out << padding << "\"" << entry.first << "\" : ";
        else
          _out << padding << entry.first << " : ";
        entry.second.p_write(_out, _format, _level+1);
      }
    }
    if ( !isFirst && _format.type == format_type::pretty )
      _out << endl << final_padding;
    _out << "}";
  }
  else if ( is_array() )
  {
    _out << "[";
    bool isFirst = true;
    for ( size_t i = 0; i < this->size(); i++ )
    {
      if ( ! isFirst )
      {
        _out << ",";
        if ( _format.type == format_type::pretty )
          _out << endl;
      }
      else if ( _format.type == format_type::pretty )
        _out << endl;
      isFirst = false;
      if ( _format.type == format_type::pretty )
        _out << padding;
      (*this)[i].p_write(_out, _format, _level+1);
    }
    if ( !isFirst && _format.type == format_type::pretty )
      _out << endl << final_padding;
    _out << "]";
  }
  else if ( is_string() )
  {
    if ( ! _format.string_no_quotes
         || ( m_data._str.length() == 4 && (m_data._str == "true" || m_data._str == "null") )
         || ( m_data._str.length() == 5 && m_data._str == "false") )
     _out << "\"" << escape_string(m_data._str) << "\"";
    else
     _out << escape_string(m_data._str);
  }
  else if ( is_null() )
    _out << "null";
  else
    _out << this->as_str();
}

const value& value::operator[](const size_t _index) const
{
  if ( ! is_array() )
    throw std::runtime_error(__func__ + std::string(": can be used only for array type"));
  if ( _index >= m_data._arr.size() )
    throw std::runtime_error(__func__ + std::string(": index(") + std::to_string(_index)
                         + ") out of range(" + std::to_string(m_data._arr.size()) + ")");
  return m_data._arr[_index];
}

value& value::operator[](const size_t _index)
{
  if ( ! is_array() )
    throw std::runtime_error(__func__ + std::string(": can be used only for array type"));
  if ( _index >= m_data._arr.size() )
    throw std::runtime_error(__func__ + std::string(": index(") + std::to_string(_index)
                         + ") out of range(" + std::to_string(m_data._arr.size()) + ")");
  return m_data._arr[_index];
}

const value& value::operator[](const std::string& _key) const
{
  if ( ! is_object() )
    throw std::runtime_error(__func__ + std::string(": can be used only for object type"));
  for ( const auto& entry : m_data.map() )
    if ( entry.first == _key ) return entry.second;
  throw std::runtime_error(__func__ + std::string(": key(") + _key + ") not found");
}

value& value::operator[](const std::string& _key)
{
  if ( ! is_object() )
  {
    this->clear();
    m_type = m_data.init(value_type::object);
  }
  return (m_data.map())[_key];
}

// Erase value from the array
void value::erase(const std::string& _key)
{
  if ( ! is_object() )
    throw std::runtime_error(__func__ + std::string("key can be used only for object type"));
  m_data.map().erase(_key);
}

value& value::append(const value& _obj)
{
  if ( ! is_array() )
  {
    this->clear();
    m_type = m_data.init(value_type::array);
  }
  m_data._arr.push_back(_obj);
  return m_data._arr[m_data._arr.size()-1];
}

value& value::append()
{
  if ( ! is_array() )
  {
    this->clear();
    m_type = m_data.init(value_type::array);
  }
  value jval;
  m_data._arr.push_back(std::move(jval));
  return m_data._arr[m_data._arr.size()-1];
}

// Erase value from the array
void value::erase(const size_t _index)
{
  if ( ! is_array() )
    throw std::runtime_error(__func__ + std::string(": can be used only for array type"));
  if ( _index >= m_data._arr.size() )
    throw std::out_of_range(__func__ + std::string("; Attempting to delete index ") + std::to_string(_index));
  m_data._arr.erase(m_data._arr.begin() + _index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of value::union_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
value::union_data::union_data(const value_type _type)
{
  init(_type);
}

value::union_data::union_data(const union_data& _obj, const value_type _type)
{
  init(_obj, true, _type);
}

value::union_data::union_data(union_data&& _obj, const value_type _type) noexcept
{
  init(std::move(_obj), _type);
}

value::union_data::~union_data()
{
}

#include <iostream>
using namespace std;
value_type value::union_data::clear(const value_type _type)
{
  switch ( _type )
  {
  case value_type::string: _str.~string(); break;
  case value_type::array:
    // Iteratively delete all entries
    for ( auto& entry : _arr )
      entry.clear();
    _arr.~array_t();
    break;
  case value_type::object:
    // Iteratively delete all entries
    for ( auto& entry : *_map )
      entry.second.clear();
    delete _map;
    _map = nullptr;
    //--json_gobjects_alloc;
    break;
  default: break;
  }
  return value_type::null;
}

value_type value::union_data::init(const value_type _type/* = value_type::null*/)
{
  switch ( _type )
  {
  case value_type::null:      break;
  case value_type::string:    new (&_str) std::string; break;
  case value_type::_signed:   _i64 = 0; break;
  case value_type::_unsigned: _u64 = 0; break;
  case value_type::_double:   _dbl = 0; break;
  case value_type::boolean:   _bval = false; break;
  case value_type::array:     new (&_arr) array_t; break;
  case value_type::object:    _map = new object_t; /*++json_gobjects_alloc;*/ break;
  }
  return _type;
}

value_type value::union_data::init(
  const union_data& _obj,
  const bool        _new/* = true*/,
  const value_type  _type/* = value_type::null*/
  )
{
  switch ( _type )
  {
  case value_type::null:      break;
  case value_type::string:    init(_obj._str);  break;
  case value_type::_signed:   init(_obj._i64);  break;
  case value_type::_unsigned: init(_obj._u64);  break;
  case value_type::_double:   init(_obj._dbl);  break;
  case value_type::boolean:   init(_obj._bval); break;
  case value_type::array:     init(_obj._arr);  break;
  case value_type::object:    init(_obj.map(), _new); break;
  }
  return _type;
}

value_type value::union_data::init(const int _val)
{
  return this->init(static_cast<int64_t>(_val));
}

value_type value::union_data::init(const int64_t _val)
{
  _i64 = _val;
  return value_type::_signed;
}

value_type value::union_data::init(const uint64_t _val)
{
  _u64 = _val;
  return value_type::_unsigned;
}

value_type value::union_data::init(const long double _val)
{
  _dbl = _val;
  return value_type::_double;
}

value_type value::union_data::init(const bool _val)
{
  _bval = _val;
  return value_type::boolean;
}

value_type value::union_data::init(const std::string& _val)
{
  new (&_str) std::string(_val);
  return value_type::string;
}

value_type value::union_data::init(const char* _val)
{
  if ( _val != nullptr )
    return init(std::string(_val));
  return init(value_type::null);
}

value_type value::union_data::init(const array_t& _val)
{
  new (&_arr) array_t(_val);
  return value_type::array;
}

value_type value::union_data::init(const object_t& _val, const bool _new/* = true*/)
{
  if ( _new )
  {
    _map = new object_t(_val);
    /*++json_gobjects_alloc;*/
  }
  else
    *_map = _val;
  return value_type::object;
}

//! Move initializer routine
value_type value::union_data::init(union_data&& _obj, value_type _type) noexcept
{
  switch ( _type )
  {
  case value_type::null:            break;
  case value_type::string:          new (&_str) std::string(_obj._str); break;
  case value_type::_signed:         _i64  = _obj._i64;  break;
  case value_type::_unsigned:       _u64  = _obj._u64;  break;
  case value_type::_double:         _dbl  = _obj._dbl;  break;
  case value_type::boolean:         _bval = _obj._bval; break;
  case value_type::array:           new (&_arr) array_t(_obj._arr); break;
  case value_type::object:          _map  = _obj._map;  break;
  }
  // We've made a copy of the string and array objects.
  // So, we need to clear them from _obj
  // object uses a pointer. We don't want to clear the object type alone
  if ( _type != value_type::object )
    _obj.clear(_type);
  return _type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of value_type
//
///////////////////////////////////////////////////////////////////////////////////////////////////
struct json_value_type_map
{
  value_type  type;
  const char* name;
}
  static gValueTypeMap[] =
  {
    {value_type::null,      "null"},
    {value_type::string,    "string"},
    {value_type::_signed,   "signed"},
    {value_type::_unsigned, "unsigned"},
    {value_type::_double,   "double"},
    {value_type::boolean,   "boolean"},
    {value_type::object,    "object"},
    {value_type::array,     "array"}
  };

std::string json::to_str(const json::value_type& _type)
{
  std::string name;
  for ( const auto& entry : gValueTypeMap )
  {
    if ( _type == entry.type )
    {
      name = entry.name;
      break;
    }
  }
  return name;
}
