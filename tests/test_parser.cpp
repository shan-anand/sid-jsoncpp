#include <gtest/gtest.h>
#include "json/json.h"

using namespace sid::json;

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ParserTest, ParseSimpleObject) {
    std::string json = R"({"name": "John", "age": 30})";
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_TRUE(out.jroot.is_object());
    EXPECT_EQ(out.jroot["name"].get_str(), "John");
    EXPECT_EQ(out.jroot["age"].get_int64(), 30);
    EXPECT_NO_THROW(out.clear());
}

TEST_F(ParserTest, ParseSimpleArray) {
    std::string json = R"([1, "hello", true, null, false])";
    parser_input in; // Set using in.set() below
    parser_output out;
    
    EXPECT_NO_THROW(in.set(input_type::data, json));
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_TRUE(out.jroot.is_array());
    EXPECT_EQ(out.jroot.size(), 5);
    EXPECT_EQ(out.jroot[0].get_int64(), 1);
    EXPECT_EQ(out.jroot[1].get_str(), "hello");
    EXPECT_EQ(out.jroot[2].get_bool(), true);
    EXPECT_TRUE(out.jroot[3].is_null());
    EXPECT_EQ(out.jroot[4].get_bool(), false);
}

TEST_F(ParserTest, ParseNestedStructures) {
    std::string json = R"({
        "user": {
            "name": "Alice",
            "scores": [95, 87, 92]
        },
        "active": true
    })";
    
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_TRUE(out.jroot.is_object());
    EXPECT_EQ(out.jroot["user"]["name"].get_str(), "Alice");
    EXPECT_TRUE(out.jroot["user"]["scores"].is_array());
    EXPECT_EQ(out.jroot["user"]["scores"].size(), 3);
    EXPECT_EQ(out.jroot["user"]["scores"][0].get_int64(), 95);
    EXPECT_EQ(out.jroot["active"].get_bool(), true);
}

TEST_F(ParserTest, ParseWithStats) {
    std::string json = R"({"obj": {}, "arr": [1, 2], "str": "test"})";
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_GT(out.stats.objects, 0);
    EXPECT_GT(out.stats.arrays, 0);
    EXPECT_GT(out.stats.strings, 0);
    EXPECT_GT(out.stats.keys, 0);
    EXPECT_GT(out.stats.time_ms, 0);
}

TEST_F(ParserTest, ParseWithControl) {
    std::string json = R"({key: "value1", key: "value2"})";
    parser_input in {input_type::data, json};
    in.ctrl.mode.allowFlexibleKeys = true;
    in.ctrl.dupKey = parser_control::dup_key::append;
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_TRUE(out.jroot["key"].is_array());
    EXPECT_EQ(out.jroot["key"].size(), 2);
}

TEST_F(ParserTest, ParseNumbers) {
    std::string json = R"({
        "int": 42,
        "negative": -17,
        "float": 3.14,
        "exp": 1.23e-4
    })";
    
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_EQ(out.jroot["int"].get_int64(), 42);
    EXPECT_EQ(out.jroot["negative"].get_int64(), -17);
    EXPECT_DOUBLE_EQ(out.jroot["float"].get_double(), 3.14);
    EXPECT_NEAR(out.jroot["exp"].get_double(), 1.23e-4, 1e-10);
}

TEST_F(ParserTest, ParseEscapedStrings) {
    std::string json = R"({"text": "Hello\nWorld\t\"Quote\""})";
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_EQ(out.jroot["text"].get_str(), "Hello\nWorld\t\"Quote\"");
}

TEST_F(ParserTest, ParseComments) {
    std::string json = R"({
        // C++ style comment
        "name": "test", /* C style comment */
        # Shell style comment
        "value": 42
    })";
    
    parser_input in {input_type::data, json};
    parser_output out;
    
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_EQ(out.jroot["name"].get_str(), "test");
    EXPECT_EQ(out.jroot["value"].get_int64(), 42);
}

TEST_F(ParserTest, ParseErrors) {
    parser_input in;
    parser_output out;
    
    // Invalid JSON should throw exceptions
    in = {input_type::data, "{invalid}"};
    EXPECT_THROW(value::parse(in, out), std::exception);
    
    in = {input_type::data, "[1, 2,]"};
    EXPECT_THROW(value::parse(in, out), std::exception);
    
    in = {input_type::data, R"({"key": })"};
    EXPECT_THROW(value::parse(in, out), std::exception);
    
    // Empty input should throw
    in = {input_type::data, ""};
    EXPECT_THROW(value::parse(in, out), std::exception);
    
    in = {input_type::data, "   "};
    EXPECT_THROW(value::parse(in, out), std::exception);
}

TEST_F(ParserTest, DuplicateKeyHandling) {
    std::string json = R"({"key": "first", "key": "second"})";
    parser_input in {input_type::data, json};
    parser_output out;
    
    // Accept (default)
    in.ctrl.dupKey = parser_control::dup_key::overwrite;
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_EQ(out.jroot["key"].get_str(), "second");
    
    // Ignore
    in.ctrl.dupKey = parser_control::dup_key::ignore;
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_EQ(out.jroot["key"].get_str(), "first");
    
    // Append
    in.ctrl.dupKey = parser_control::dup_key::append;
    EXPECT_NO_THROW(value::parse(in, out));
    EXPECT_TRUE(out.jroot["key"].is_array());
    EXPECT_EQ(out.jroot["key"].size(), 2);
    
    // Reject should throw exception
    in.ctrl.dupKey = parser_control::dup_key::reject;
    EXPECT_THROW(value::parse(in, out), std::exception);
}