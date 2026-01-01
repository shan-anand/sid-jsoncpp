/*
LICENSE: BEGIN
===============================================================================
@author Shan Anand
@email anand.gs@gmail.com
@source https://github.com/shan-anand
@file test_schema.cpp
@brief Schema tests
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
 * @file  test_schema.cpp
 * @brief Schema tests
 */
#include <gtest/gtest.h>
#include "json/json.h"

using namespace sid::json;

class SchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SchemaTest, SchemaTypeBasics) {
    schema_type null_type(schema_type::null);
    EXPECT_EQ(null_type.id(), schema_type::null);
    EXPECT_EQ(null_type.name(), "null");
    EXPECT_TRUE(null_type.empty());
    EXPECT_FALSE(null_type.is_container());
    
    schema_type obj_type(schema_type::object);
    EXPECT_EQ(obj_type.id(), schema_type::object);
    EXPECT_EQ(obj_type.name(), "object");
    EXPECT_FALSE(obj_type.empty());
    EXPECT_TRUE(obj_type.is_container());
    
    schema_type arr_type(schema_type::array);
    EXPECT_TRUE(arr_type.is_container());
    arr_type.clear();
    arr_type = schema_type::array;
    EXPECT_EQ(arr_type.id(), schema_type::array);
}

TEST_F(SchemaTest, SchemaTypeFromString) {
    schema_type type;
    
    EXPECT_TRUE(schema_type::get("string", type));
    EXPECT_EQ(type.id(), schema_type::string);
    
    EXPECT_TRUE(schema_type::get("number", type));
    EXPECT_EQ(type.id(), schema_type::number);
    
    EXPECT_TRUE(schema_type::get("boolean", type));
    EXPECT_EQ(type.id(), schema_type::boolean);
    
    EXPECT_FALSE(schema_type::get("invalid", type));
}

TEST_F(SchemaTest, SchemaTypeComparison) {
    schema_type str1(schema_type::string);
    schema_type str2(schema_type::string);
    schema_type num(schema_type::number);
    
    EXPECT_TRUE(str1 == schema_type::string);
    EXPECT_FALSE(str1 == schema_type::number);
    EXPECT_TRUE(str1 < num);
}

TEST_F(SchemaTest, SchemaTypes) {
    schema_types types;
    
    types.add(schema_type::string);
    types.add(schema_type::number);
    
    EXPECT_TRUE(types.exists(schema_type::string));
    EXPECT_TRUE(types.exists(schema_type::number));
    EXPECT_FALSE(types.exists(schema_type::boolean));
    
    types.remove(schema_type::string);
    EXPECT_FALSE(types.exists(schema_type::string));
    EXPECT_TRUE(types.exists(schema_type::number));
}

TEST_F(SchemaTest, SchemaTypesFromValue) {
    schema_types types;
    
    value str_val("string");
    types.add(str_val);
    EXPECT_TRUE(types.exists(schema_type::string));
}

TEST_F(SchemaTest, PropertyBasics) {
    schema::property prop;
    
    EXPECT_TRUE(prop.key.empty());
    EXPECT_TRUE(prop.description.empty());
    EXPECT_TRUE(prop.type.empty());
    EXPECT_FALSE(prop.minimum.has_value());
    EXPECT_FALSE(prop.maximum.has_value());
    
    prop.key = "test_key";
    prop.description = "Test property";
    prop.type.add(schema_type::string);
    prop.minLength = 5;
    prop.maxLength = 100;
    
    EXPECT_EQ(prop.key, "test_key");
    EXPECT_EQ(prop.description, "Test property");
    EXPECT_TRUE(prop.type.exists(schema_type::string));
    EXPECT_EQ(prop.minLength.value(), 5);
    EXPECT_EQ(prop.maxLength.value(), 100);
}

TEST_F(SchemaTest, SchemaBasics) {
    schema s;
    
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s.title.empty());
    EXPECT_TRUE(s.description.empty());
    EXPECT_TRUE(s.type.empty());
    
    s.title = "Test Schema";
    s.description = "A test schema";
    s.type.add(schema_type::object);
    
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(s.title, "Test Schema");
    EXPECT_EQ(s.description, "A test schema");
    EXPECT_TRUE(s.type.exists(schema_type::object));
}

TEST_F(SchemaTest, SchemaRequired) {
    schema s;
    
    s.required.insert("name");
    s.required.insert("age");
    
    EXPECT_EQ(s.required.size(), 2);
    EXPECT_TRUE(s.required.find("name") != s.required.end());
    EXPECT_TRUE(s.required.find("age") != s.required.end());
    EXPECT_TRUE(s.required.find("email") == s.required.end());
}

TEST_F(SchemaTest, SchemaFromJson) {
    std::string schema_json = R"({
        "$schema": "http://json-schema.org/draft-07/schema#",
        "title": "Person",
        "type": "object",
        "properties": {
            "name": {
                "type": "string",
                "minLength": 1
            },
            "age": {
                "type": "integer",
                "minimum": 0
            }
        },
        "required": ["name"]
    })";
    
    parser_output out;
    
    try {
        value::parse(out, schema_json);
        schema s = schema::parse(out.jroot);
        
        EXPECT_EQ(s.title, "Person");
        EXPECT_TRUE(s.type.exists(schema_type::object));
        EXPECT_EQ(s.required.size(), 1);
        EXPECT_TRUE(s.required.find("name") != s.required.end());
    } catch (const std::exception& e) {
        FAIL() << "Schema parsing should not throw: " << e.what();
    }
}

TEST_F(SchemaTest, SchemaClear) {
    schema s;
    s.title = "Test";
    s.description = "Description";
    s.type.add(schema_type::object);
    s.required.insert("field");
    
    EXPECT_FALSE(s.empty());
    
    s.clear();
    
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s.title.empty());
    EXPECT_TRUE(s.description.empty());
    EXPECT_TRUE(s.type.empty());
    EXPECT_TRUE(s.required.empty());
}

TEST_F(SchemaTest, PropertyConstraints) {
    schema::property prop;
    
    // Number constraints
    prop.minimum = 10;
    prop.maximum = 100;
    prop.exclusiveMinimum = 5;
    prop.exclusiveMaximum = 105;
    prop.multipleOf = 5;
    
    EXPECT_EQ(prop.minimum.value(), 10);
    EXPECT_EQ(prop.maximum.value(), 100);
    EXPECT_EQ(prop.exclusiveMinimum.value(), 5);
    EXPECT_EQ(prop.exclusiveMaximum.value(), 105);
    EXPECT_EQ(prop.multipleOf.value(), 5);
    
    // Array constraints
    prop.minItems = 1;
    prop.maxItems = 10;
    prop.uniqueItems = true;
    prop.minContains = 2;
    prop.maxContains = 8;
    
    EXPECT_EQ(prop.minItems.value(), 1);
    EXPECT_EQ(prop.maxItems.value(), 10);
    EXPECT_TRUE(prop.uniqueItems.value());
    EXPECT_EQ(prop.minContains.value(), 2);
    EXPECT_EQ(prop.maxContains.value(), 8);
    
    // Object constraints
    prop.minProperties = 1;
    prop.maxProperties = 5;
    
    EXPECT_EQ(prop.minProperties.value(), 1);
    EXPECT_EQ(prop.maxProperties.value(), 5);
}