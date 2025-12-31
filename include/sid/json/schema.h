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
#include <set>
#include <optional>
#include <cstdint>
#include "value.h"

namespace sid::json {

/**
 * @class schema_type
 * @brief json schema_type
 */
struct schema_type
{
  enum ID : uint8_t {
    null, object, array, string, boolean, number, integer
  };

public:
  ID id() const { return m_id; }
  std::string name() const;

  static bool get(const std::string& _name, /*out*/ schema_type& _type);
  static schema_type get(const std::string& _name);

public:
  schema_type() : m_id(schema_type::null) {}
  schema_type(const schema_type&) = default;
  schema_type(const schema_type::ID& _id) : m_id(_id) {}

  schema_type& operator=(const schema_type&) = default;
  schema_type& operator=(const schema_type::ID& _id) { m_id = _id; return *this; }
  //bool operator==(const schema_type& _obj) const = { return (m_id == _obj.m_id); }
  bool operator==(const schema_type::ID& _id) const { return (m_id == _id); }
  bool operator<(const schema_type& _obj) const { return (m_id < _obj.m_id); }

  void clear() { m_id = schema_type::null; }
  bool empty() const { return (m_id == schema_type::null); }
  bool is_container() const {
    return (m_id == schema_type::object || m_id == schema_type::array);
  }

private:
  ID m_id;
};

struct schema_types : public std::set<schema_type>
{
  void add(const schema_type& _type) { this->insert(_type); }
  void add(const schema_type::ID& _typeId) { return add(schema_type(_typeId)); }
  void add(const value& _value);
  bool exists(const schema_type& _type) const { return this->find(_type) != this->end(); }
  void remove(const schema_type& _type) { this->erase(_type); }
  value to_json() const;
};

/**
 * @class schema
 * @brief json schema class
 */
class schema
{
public:
  struct property;

  struct property_vec : public std::vector<property>
  {
    void set(const value& _jproperties);
    std::string to_string() const;
    value to_json() const;
  };

  struct property
  {
    std::string             key;
    std::string             description;
    schema_types            type;
    // For numbers
    std::optional<int64_t>  minimum;
    std::optional<int64_t>  exclusiveMinimum;
    std::optional<int64_t>  maximum;
    std::optional<int64_t>  exclusiveMaximum;
    std::optional<int64_t>  multipleOf;
    // For strings
    std::optional<size_t>   minLength;
    std::optional<size_t>   maxLength;
    std::string             pattern;
    // For arrays
    std::optional<size_t>   minItems;
    std::optional<size_t>   maxItems;
    std::optional<bool>     uniqueItems;
    std::optional<size_t>   minContains;
    std::optional<size_t>   maxContains;
    // For objects
    std::optional<size_t>   minProperties;
    std::optional<size_t>   maxProperties;
    std::set<std::string>   required;
    property_vec            properties;

    property();
    void clear();
    void set(const value& _jproperties, const std::string& _key);
    std::string to_string() const;
    value to_json() const;
  };

  //! Schema members
  std::string           _schema;     //! Json schema URI
  std::string           _id;         //! Identifier
  std::string           title;       //! Schema title
  std::string           description; //! Description of the schema
  schema_types          type;        //! Schema type
  property_vec          properties;  //! Properties associated with the schema
  std::set<std::string> required;    //! Required properties

  schema();
  void clear();
  bool empty() const;
  std::string to_string() const;
  value to_json() const;

  static schema parse_file(const std::string& _schemaFile);
  static schema parse(const std::string& _schemaData);
  static schema parse(const value& _jroot);
};

} // namespace sid::json
