/*
LICENSE: BEGIN
===============================================================================
@author Shan Anand
@email anand.gs@gmail.com
@source https://github.com/shan-anand
@file json.cpp
@brief Json schema handling using c++
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
 * @file  schema.cpp
 * @brief Implementation of json schema
 */
#include "json/schema.h"
#include "parser_io.h"
#include "parser.h"
#include <fstream>
#include <stack>
#include <iomanip>
#include <cstring>
#include <unistd.h>

using namespace sid::json;

namespace local
{
void fill_required(
  std::set<std::string>&      _required,
  const value&                _jarray,
  const schema::property_vec& _properties
  );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of schema::property
//
///////////////////////////////////////////////////////////////////////////////////////////////////
schema::property::property()
{
}

void schema::property::clear()
{
  key.clear();
  description.clear();
  type.clear();
  // For numbers
  minimum.reset();
  exclusiveMinimum.reset();
  maximum.reset();
  exclusiveMaximum.reset();
  multipleOf.reset();
  // For strings
  minLength.reset();
  maxLength.reset();
  pattern.clear();
  // For arrays
  minItems.reset();
  maxItems.reset();
  uniqueItems.reset();
  minContains.reset();
  maxContains.reset();
  // For objects
  minProperties.reset();
  maxProperties.reset();
  required.clear();
  properties.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of schema
//
///////////////////////////////////////////////////////////////////////////////////////////////////
schema::schema()
{
  clear();
}

void schema::clear()
{
  this->_schema = "https://json-schema.org/draft/2020-12/schema";
  //_schema.clear();
  _id.clear();
  title.clear();
  description.clear();
  type.clear();
  properties.clear();
  required.clear();
}

bool schema::empty() const
{
  for ( const schema_type& t : this->type )
    if ( ! t.is_container() )
      return true;
  return this->type.empty();
}

/*static*/
schema schema::parse_file(const std::string& _schemaFile)
{
  std::ifstream in;
  in.open(_schemaFile);
  if ( ! in.is_open() )
    throw std::runtime_error("Failed to open schema file: " + _schemaFile);
  char buf[8096] = {0};
  std::string jsonStr;
  while ( ! in.eof() )
  {
    ::memset(buf, 0, sizeof(buf));
    in.read(buf, sizeof(buf)-1);
    if ( in.bad() )
      throw std::system_error(errno, std::system_category(), "Failed to read schema file");
    jsonStr += buf;
  }
  return parse(jsonStr);
}

/*static*/
schema schema::parse(const std::string& _schemaData)
{
  char_parser_input in(_schemaData, input_type::data);
  parser_output out;
  char_parser parser(in, out);
  parser.parse();
  return parse(out.jroot);
}

/*static*/
schema schema::parse(const value& _jroot)
{
  schema schema;
  value jval;
  if ( _jroot.has_key("$schema", jval) && !jval.is_null() )
    schema._schema = jval.get_str();
  if ( _jroot.has_key("$id", jval) && !jval.is_null() )
    schema._id = jval.get_str();
  if ( _jroot.has_key("title", jval) && !jval.is_null() )
    schema.title = jval.get_str();
  if ( _jroot.has_key("description", jval) && !jval.is_null() )
    schema.description = jval.get_str();

  if ( !_jroot.has_key("type", jval) )
    throw std::runtime_error("type missing in schema");

  // set the schema type
  schema.type.add(jval);
  // Top level type must be an object or an array
  {
    schema_types type = schema.type;
    type.remove(schema_type::object);
    type.remove(schema_type::array);
    if ( !type.empty() )
      throw std::runtime_error("Top-level schema type must be an object or an array");
  }

  const bool hasProperties = _jroot.has_key("properties", jval);
  if ( schema.type.exists(schema_type::object) )
  {
    if ( ! hasProperties )
      throw std::runtime_error("properties missing in schema");
    schema.properties.set(jval);
  }
  else if ( hasProperties )
    throw std::runtime_error("properties is applicable only for object type schema");

  if ( _jroot.has_key("required", jval) )
  {
    if ( ! schema.type.exists(schema_type::object) )
      throw std::runtime_error("required is applicable only for object type schema");
    local::fill_required(schema.required, jval, schema.properties);
  }
  return schema;
}

void schema::property_vec::set(const value& _jproperties)
{
  if ( ! _jproperties.is_object() )
    throw std::runtime_error("properties must be an object");

  for ( const std::string& key : _jproperties.get_keys() )
  {
    schema::property property;
    property.set(_jproperties, key);
    this->push_back(property);
  }
}

void schema::property::set(const value& _jproperties, const std::string& _key)
{
  const value& jproperty = _jproperties[_key];
  value jval;

  this->key = _key;
  if ( ! jproperty.has_key("type", jval) )
    throw std::runtime_error("property type missing for " + this->key);
  this->type.add(jval);
  if ( jproperty.has_key("description", jval) && !jval.is_null() )
    this->description = jval.get_str();

  if ( this->type.exists(schema_type::number) || this->type.exists(schema_type::integer) )
  {
    if ( jproperty.has_key("minimum", jval) )
    {
      if ( ! jval.is_decimal() )
        throw std::runtime_error("minimum must be a decimal value");
      this->minimum = jval.get_int64();
    }
    if ( jproperty.has_key("exclusiveMinimum", jval) )
    {
      if ( ! jval.is_decimal() )
        throw std::runtime_error("exclusiveMinimum must be a decimal value");
      this->minimum = jval.get_int64();
    }
    if ( jproperty.has_key("maximum", jval) )
    {
      if ( ! jval.is_decimal() )
        throw std::runtime_error("maximum must be a decimal value");
      this->minimum = jval.get_int64();
    }
    if ( jproperty.has_key("exclusiveMaximum", jval) )
    {
      if ( ! jval.is_decimal() )
        throw std::runtime_error("exclusiveMaximum must be a decimal value");
      this->minimum = jval.get_int64();
    }
    if ( jproperty.has_key("multipleOf", jval) )
    {
      if ( ! jval.is_decimal() )
        throw std::runtime_error("multipleOf must be a decimal value");
      this->minimum = jval.get_int64();
    }
  }
  if ( this->type.exists(schema_type::string) )
  {
    if ( jproperty.has_key("minLength", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("minLength must be an unsigned value");
      this->minLength = jval.get_uint64();
    }
    if ( jproperty.has_key("maxLength", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("maxLength must be an unsigned value");
      this->maxLength = jval.get_uint64();
    }
    if ( jproperty.has_key("pattern", jval) )
    {
      if ( ! jval.is_string() )
        throw std::runtime_error("pattern must be a string");
      this->pattern = jval.get_str();
    }
  }
  if ( this->type.exists(schema_type::array) )
  {
    if ( jproperty.has_key("minItems", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("minItems must be an unsigned value");
      this->minItems = jval.get_uint64();
    }
    if ( jproperty.has_key("maxItems", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("maxItems must be an unsigned value");
      this->maxItems = jval.get_uint64();
    }
    if ( jproperty.has_key("uniqueItems", jval) )
    {
      if ( ! jval.is_bool() )
        throw std::runtime_error("uniqueItems must be a boolean value");
      this->uniqueItems = jval.get_bool();
    }
    if ( jproperty.has_key("minContains", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("minContains must be an unsigned value");
      this->minContains = jval.get_uint64();
    }
    if ( jproperty.has_key("maxContains", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("maxContains must be an unsigned value");
      this->maxContains = jval.get_uint64();
    }
  }
  if ( this->type.exists(schema_type::object) )
  {
    if ( jproperty.has_key("minProperties", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("minProperties must be an unsigned value");
      this->minProperties = jval.get_uint64();
    }
    if ( jproperty.has_key("maxProperties", jval) )
    {
      if ( ! jval.is_unsigned() )
        throw std::runtime_error("maxProperties must be an unsigned value");
      this->maxProperties = jval.get_uint64();
    }
  }
  if ( jproperty.has_key("properties", jval) )
  {
    if ( ! this->type.exists(schema_type::object) )
      throw std::runtime_error("properties is applicable only for object types. Key: " + this->key);
    this->properties.set(jval);
  }
  if ( jproperty.has_key("required", jval) )
  {
    if ( ! this->type.exists(schema_type::object) )
      throw std::runtime_error("required is applicable only for object types for key " + this->key);
    local::fill_required(this->required, jval, this->properties);
  }
}

std::string schema::to_str() const
{
  return to_json().to_str(format_type::pretty);
}

value schema::to_json() const
{
  value jroot;
  if ( ! this->_schema.empty() )
    jroot["$schema"] = this->_schema;
  if ( ! this->_id.empty() )
    jroot["$id"] = this->_id;
  if ( ! this->title.empty() )
    jroot["title"] = this->title;
  if ( ! this->description.empty() )
    jroot["description"] = this->description;
  if ( this->type.empty() )
    throw std::runtime_error("Schema type not set");
  jroot["type"] = this->type.to_json();
  if ( ! this->properties.empty() )
    jroot["properties"] = this->properties.to_json();
  for ( const std::string& req :this->required )
    jroot["required"].append(req);
  
  return jroot;
}

std::string schema::property_vec::to_str() const
{
  return to_json().to_str(format_type::pretty);
}

value schema::property_vec::to_json() const
{
  value jroot;

  for ( const schema::property& property : *this )
    jroot[property.key] = property.to_json();
  return jroot;
}

std::string schema::property::to_str() const
{
  return to_json().to_str(format_type::pretty);
}

value schema::property::to_json() const
{
  value jroot;
  
  if ( ! this->description.empty() )
    jroot["description"] = this->description;
  if ( this->type.empty() )
    throw std::runtime_error("Property type not set");
  jroot["type"] = this->type.to_json();

  if ( this->type.exists(schema_type::number) || this->type.exists(schema_type::integer) )
  {
    if ( this->minimum )
      jroot["minimum"] = this->minimum.value();
    if ( this->exclusiveMinimum )
      jroot["exclusiveMinimum"] = this->exclusiveMinimum.value();
    if ( this->maximum )
      jroot["maximum"] = this->maximum.value();
    if ( this->exclusiveMaximum )
      jroot["exclusiveMaximum"] = this->exclusiveMaximum.value();
    if ( this->multipleOf )
      jroot["multipleOf"] = this->maximum.value();
  }
  if ( this->type.exists(schema_type::string) )
  {
    if ( this->minLength )
      jroot["minLength"] = this->minLength.value();
    if ( this->maxLength )
      jroot["maxLength"] = this->maxLength.value();
    if ( ! this->pattern.empty() )
      jroot["pattern"] = this->pattern;
  }
  if ( this->type.exists(schema_type::array) )
  {
    if ( this->minItems )
      jroot["minItems"] = this->minItems.value();
    if ( this->maxItems )
      jroot["maxItems"] = this->maxItems.value();
    if ( this->uniqueItems )
      jroot["uniqueItems"] = this->uniqueItems.value();
    if ( this->minContains )
      jroot["minContains"] = this->minContains.value();
    if ( this->maxContains )
      jroot["maxContains"] = this->maxContains.value();
  }
  if ( this->type.exists(schema_type::object) )
  {
    if ( this->minProperties )
      jroot["minProperties"] = this->minProperties.value();
    if ( this->maxProperties )
      jroot["maxProperties"] = this->maxProperties.value();
    if ( ! this->properties.empty() )
      jroot["jproperties"] = this->properties.to_json();
    for ( const std::string& req : this->required )
      jroot["required"].append(req);
  }
  return jroot;
}

value schema_types::to_json() const
{
  value jroot;
  if ( this->size() == 1 )
    jroot = (*(this->begin())).name();
  else
  {
    for ( const schema_type& type : *this )
      jroot.append(type.name());
  }
  return jroot;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of schema_type
//
///////////////////////////////////////////////////////////////////////////////////////////////////
struct json_schema_type_map
{
  schema_type::ID type;
  const char*           name;
}
  static gSchemaTypeMap[] =
  {
    {schema_type::null,    "null"},
    {schema_type::object,  "object"},
    {schema_type::array,   "array"},
    {schema_type::string,  "string"},
    {schema_type::boolean, "boolean"},
    {schema_type::number,  "number"},
    {schema_type::integer, "integer"}
  };

/*static*/
bool schema_type::get(const std::string& _name, /*out*/ schema_type& _type)
{
  for ( const auto& entry : gSchemaTypeMap )
  {
    if ( _name == entry.name )
    {
      _type = entry.type;
      return true;
    }
  }
  return false;
}

/*static*/
schema_type schema_type::get(const std::string& _name)
{
  schema_type type;
  if ( !get(_name, /*out*/ type) )
    throw std::runtime_error("Invalid schema type [" + _name + "] encountered");
  return type;
}

std::string schema_type::name() const
{
  std::string name;
  for ( const auto& entry : gSchemaTypeMap )
  {
    if ( m_id == entry.type )
    {
      name = entry.name;
      break;
    }
  }
  return name;
}

void schema_types::add(const value& _value)
{
  if ( _value.is_string() )
    this->insert(schema_type::get(_value.get_str()));
  else if ( _value.is_array() )
  {
    // Each array element must be a string
    for ( size_t i = 0; i < _value.size(); i++ )
    {
      const value& jval = _value[i];
      if ( !jval.is_string() )
        throw std::runtime_error("type parameter must be strings within the array");
      schema_type type = schema_type::get(jval.get_str());
      if ( this->exists(type) )
        throw std::runtime_error("type parameters must be unique within the array");
      this->insert(type);
    }
  }
  else
    throw std::runtime_error("type parameter must be string or an array of unique string");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of local namespace
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void local::fill_required(
  std::set<std::string>&      _required,
  const value&                _jarray,
  const schema::property_vec& _properties
  )
{
  if ( ! _jarray.is_array() )
    throw std::runtime_error("required must be an array of strings");

  for ( size_t i = 0; i < _jarray.size(); i++ )
  {
    const value& jval = _jarray[i];
    if ( !jval.is_string() )
      throw std::runtime_error("required parameter must be strings within the array");

    const std::string key = jval.get_str();
    if ( _required.find(key) == _required.end() )
    {
      bool found = false;
      for ( const schema::property& property : _properties )
      {
        if ( key == property.key )
        {
          found = true;
          break;
        }
      }
      if ( ! found )
        throw std::runtime_error("key (" + key + ") marked as required is not found in properties");
      _required.insert(key);
    }
  }
}
