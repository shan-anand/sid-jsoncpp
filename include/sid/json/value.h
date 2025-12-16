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

#include "format.h"
#include "parser_control.h"
#include "parser_stats.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <stdexcept>

namespace sid::json {

//! json value type
enum class value_type : uint8_t {
  null, object, array, string, boolean, _signed, _unsigned, _double
};
std::string to_str(const value_type& _type);

enum class input_type : uint8_t { data, file_path };

//! Forward declaration of json schema
class schema;
//! Forward declaration of parser (not exposed)
struct parser;
//! Forward declaration of parser_input
struct parser_input;
//! Forward declaration of parser_output
struct parser_output;

#pragma pack(push)
#pragma pack(1)

/**
 * @class value
 * @brief json value class
 */
class value
{
  friend class parser;
public:
  //! Array type definition
  using array_t = std::vector<value>;
  //! Object type definition
  using object_t = std::map<std::string, value>;
public:
  static bool parse(
    const parser_input& _in,
    parser_output&      _out
    );
  /**
   * @fn bool parse(value&                _jout,
   *                parser_stats&         _stats,
   *                const std::string&    _value,
   *                const schema&         _schema,
   *                const parser_control& _ctrl = parser_control()
   *               );
   * @brief Convert the given json string to json object
   *
   * @param _jout [out] json output
   * @param _stats [out] Parser statistics
   * @param _value [in] Input json string
   * @param _schema [in] Json schema to validate against
   * @param _ctrl [in] Parser control flags
   */
  static bool parse(
    value&                _jout,
    const std::string&    _value,
    const parser_control& _ctrl = parser_control()
    );
  static bool parse(
    value&                _jout,
    const std::string&    _value,
    const schema&         _schema,
    const parser_control& _ctrl = parser_control()
    );
  static bool parse(
    value&                _jout,
    parser_stats&         _stats,
    const std::string&    _value,
    const parser_control& _ctrl = parser_control()
    );
  static bool parse(
    value&                _jout,
    parser_stats&         _stats,
    const std::string&    _value,
    const schema&         _schema,
    const parser_control& _ctrl = parser_control()
    );

  // Constructors
  value(const value_type _type = value_type::null);
  value(const int64_t _val);
  value(const uint64_t _val);
  value(const double _val);
  value(const long double _val);
  value(const bool _val);
  value(const std::string& _val);
  value(const char* _val);
  value(const int _val);
  // Copy constructor
  value(const value& _obj);
  // Move constructor
  value(value&& _obj) noexcept;

  // Destructor
  ~value();

  bool empty() const { return m_type == value_type::null; }
  void clear();

  //! get the value_type
  value_type type() const { return m_type; }
  //! value_type check as functions
  bool is_null() const { return m_type == value_type::null; }
  bool is_string() const { return m_type == value_type::string; }
  bool is_signed() const { return m_type == value_type::_signed; }
  bool is_unsigned() const { return m_type == value_type::_unsigned; }
  bool is_decimal() const { return is_signed() || is_unsigned(); }
  bool is_double() const { return m_type == value_type::_double; }
  bool is_num() const { return is_decimal() || is_double(); }
  bool is_bool() const { return m_type == value_type::boolean; }
  bool is_array() const { return m_type == value_type::array; }
  bool is_object() const { return m_type == value_type::object; }
  bool is_basic_type() const { return ! ( is_array() || is_object() ); }
  bool is_complex_type() const { return ( is_array() || is_object() ); }

  // operator= overloads
  value& operator=(const value& _obj);
  value& operator=(const int64_t _val);
  value& operator=(const uint64_t _val);
  value& operator=(const double _val);
  value& operator=(const long double _val);
  value& operator=(const bool _val);
  value& operator=(const std::string& _val);
  value& operator=(const char* _val);
  value& operator=(const int _val);

  bool has_index(const size_t _index) const;
  bool has_key(const std::string& _key) const;
  bool has_key(const std::string& _key, value& _obj) const;
  std::vector<std::string> get_keys() const;
  size_t size() const; // For array and object type

  //! get functions
  const object_t& get_object() const;
  const array_t& get_array() const;
  int64_t get_int64() const;
  uint64_t get_uint64() const;
  long double get_double() const;
  bool get_bool() const;
  std::string get_str() const;
  std::string as_str() const;

  //! get functions with arguments
  //    0 : doesn't exist
  //    1 : Exists with non-null value
  //   -1 : Exists but it has null value
  template <typename T> int get_value(T& _val) const
  {
    if ( is_null() )
      return -1;
    if ( ! is_num() )
      throw std::runtime_error(__func__ + std::string("() can be used only for number type"));

    if ( std::is_floating_point<T>::value )
      _val = static_cast<T>(get_double());
    else if ( std::is_signed<T>::value )
      _val = static_cast<T>(get_int64());
    else
      _val = static_cast<T>(get_uint64());
    return 1;
  }
  int get_value(bool& _val) const;
  int get_value(std::string& _val) const;

  int get_value(const std::string& _key, value& _obj) const
  {
    return has_key(_key, _obj)? ( ! _obj.is_null()? 1 : -1 ) : 0;
  }

  template <typename T> int get_value(const std::string& _key, T& _val) const
  {
    value jval;
    return has_key(_key, jval)? jval.get_value(_val) : 0;
  }

  const value& operator[](const size_t _index) const;
  value& operator[](const size_t _index);
  const value& operator[](const std::string& _key) const;
  value& operator[](const std::string& _key);
  //! Append value to the array
  value& append();
  value& append(const value& _obj);
  template <typename T> value& append(const T& _val)
  {
    if ( ! is_array() )
    {
      clear();
      m_type = m_data.init(value_type::array);
    }
    value& jval = append();
    jval.m_type = jval.m_data.init(_val);
    return jval;
  }

  //! Convert json to string using the given format type
  std::string to_str(const format_type _type = format_type::compact) const;
  //! Convert json to string using the given format
  std::string to_str(const format& _format) const;

  //! Write json to the given output stream
  void write(std::ostream& _out, const format_type _type = format_type::compact) const;
  //! Write json to the given output stream using pretty format
  void write(std::ostream& _out, const format& _format) const;

private:
  void p_write(std::ostream& _out, const format& _format, uint32_t _level) const;
  void p_set(const value_type _type = value_type::null);

private:
  union union_data
  {
    int64_t     _i64;
    uint64_t    _u64;
    long double _dbl;
    bool        _bval;
    std::string _str;
    array_t     _arr;
    object_t*   _map;
    //! Default constructor
    union_data(const value_type _type = value_type::null);
    //! Copy constructor
    union_data(const union_data& _obj, const value_type _type = value_type::null);
    //! Move constructor
    union_data(union_data&& _obj, const value_type _type = value_type::null) noexcept;
    //! Destructor
    ~union_data();

    //! Clear the memory used by the object
    value_type clear(const value_type _type);

    //! Copy initializer routines
    value_type init(const value_type _type = value_type::null);
    value_type init(const union_data& _obj, const bool _new = true,
                       const value_type _type = value_type::null);
    value_type init(const int _val);
    value_type init(const int64_t _val);
    value_type init(const uint64_t _val);
    value_type init(const long double _val);
    value_type init(const bool _val);
    value_type init(const std::string& _val);
    value_type init(const char* _val);
    value_type init(const array_t& _val);
    value_type init(const object_t& _val, const bool _new = true);
    //! Move initializer routine
    value_type init(union_data&& _obj, value_type _type) noexcept;

    union_data& operator=(const union_data& _obj) { *this = std::move(_obj); return *this; }
    const object_t& map() const { return (*_map); }
    object_t& map() { return (*_map); }
  }; // union union_data

  value_type m_type; //! Type of the object
  union_data m_data; //! The object
}; // class value

#pragma pack(pop)

struct parser_input
{
  input_type     inputType;
  std::string    input;
  parser_control ctrl;
  //schema         schema;

  parser_input() {}
  parser_input(const input_type _inputType, const std::string& _input, const parser_control& _ctrl = parser_control())
    : inputType(_inputType), input(_input), ctrl(_ctrl) {}

  void set(const input_type& _inputType, const std::string& _input)
  {
    this->inputType = _inputType;
    this->input = _input;
  }
};

struct parser_output
{
  value        jroot;
  parser_stats stats;

  void clear() { jroot.clear(); stats.clear(); }
};

} // namespace sid::json
