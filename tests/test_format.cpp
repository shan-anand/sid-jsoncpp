#include <gtest/gtest.h>
#include "json/json.h"

using namespace sid::json;

class FormatTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data
        test_obj["name"] = "John";
        test_obj["age"] = 30;
        test_obj["active"] = true;
        
        test_arr.append(1);
        test_arr.append("hello");
        test_arr.append(false);
    }
    
    value test_obj;
    value test_arr;
};

TEST_F(FormatTest, FormatConstructors) {
    // Test different constructors
    format fmt1;
    EXPECT_EQ(fmt1.type, format_type::compact);
    EXPECT_FALSE(fmt1.key_no_quotes);
    EXPECT_FALSE(fmt1.string_no_quotes);
    
    format fmt2(format_type::pretty);
    EXPECT_EQ(fmt2.type, format_type::pretty);
    EXPECT_FALSE(fmt2.key_no_quotes);
    EXPECT_FALSE(fmt2.string_no_quotes);
    
    format fmt3(true, false);
    EXPECT_EQ(fmt3.type, format_type::compact);
    EXPECT_TRUE(fmt3.key_no_quotes);
    EXPECT_FALSE(fmt3.string_no_quotes);
    
    // Test format::get static method
    format fmt4 = format::get("pretty");
    EXPECT_EQ(fmt4.type, format_type::pretty);
    
    format fmt5 = format::get("compact");
    EXPECT_EQ(fmt5.type, format_type::compact);
}

TEST_F(FormatTest, FormatOptions) {
    format fmt(format_type::pretty);
    fmt.indent = 3;
    fmt.separator = '|';
    fmt.key_no_quotes = true;
    fmt.string_no_quotes = false;
    
    EXPECT_THROW(test_obj.to_str(fmt), std::runtime_error);

    fmt.separator = '\t';
    std::string result = test_obj.to_str(fmt);
    EXPECT_FALSE(result.empty());
    
    // Test with different options
    fmt.key_no_quotes = false;
    fmt.string_no_quotes = true;
    result = test_obj.to_str(fmt);
    EXPECT_FALSE(result.empty());
}

TEST_F(FormatTest, CompactFormat) {
    std::string result = test_obj.to_str();
    
    // Should not contain extra whitespace
    EXPECT_EQ(result.find('\n'), std::string::npos);
    EXPECT_EQ(result.find("  "), std::string::npos);
    
    // Should contain all data
    EXPECT_NE(result.find("\"name\""), std::string::npos);
    EXPECT_NE(result.find("\"John\""), std::string::npos);
    EXPECT_NE(result.find("30"), std::string::npos);
}

TEST_F(FormatTest, PrettyFormat) {
    format fmt(format_type::pretty);
    std::string result = test_obj.to_str(fmt);
    
    // Should contain newlines and indentation
    EXPECT_NE(result.find('\n'), std::string::npos);
    EXPECT_NE(result.find("  "), std::string::npos);
    
    // Should be properly formatted
    EXPECT_NE(result.find("{\n"), std::string::npos);
    EXPECT_NE(result.find("\n}"), std::string::npos);
}

TEST_F(FormatTest, CustomIndentation) {
    format fmt(format_type::pretty);
    fmt.indent = 4;
    
    std::string result = test_obj.to_str(fmt);
    EXPECT_NE(result.find("    "), std::string::npos); // 4 spaces
}

TEST_F(FormatTest, ArrayFormatting) {
    std::string compact = test_arr.to_str();
    EXPECT_EQ(compact.find('\n'), std::string::npos);
    
    format fmt(format_type::pretty);
    std::string pretty = test_arr.to_str(fmt);
    EXPECT_NE(pretty.find('\n'), std::string::npos);
}

TEST_F(FormatTest, EmptyContainers) {
    value empty_obj;
    // Empty object is created by accessing with []
    empty_obj["dummy"];
    empty_obj.erase("dummy");
    EXPECT_EQ(empty_obj.to_str(), "{}");
    
    value empty_arr;
    // Empty array is created by calling append then clear
    empty_arr.append(1);
    empty_arr.erase(0);
    EXPECT_EQ(empty_arr.to_str(), "[]");
}

TEST_F(FormatTest, SpecialValues) {
    value null_val;
    EXPECT_THROW(null_val.as_str(), std::exception);
    
    value bool_val(true);
    EXPECT_EQ(bool_val.as_str(), "true");
    
    value false_val(false);
    EXPECT_EQ(false_val.as_str(), "false");
}
TEST_F(FormatTest, NestedStructures) {
    value nested;
    nested["user"] = test_obj;
    nested["items"] = test_arr;
    
    format fmt(format_type::pretty);
    std::string result = nested.to_str(fmt);
    
    // Should have proper nesting
    EXPECT_NE(result.find("{\n"), std::string::npos);
    EXPECT_NE(result.find("  \"user\" : {\n"), std::string::npos);
    EXPECT_NE(result.find("  \"items\" : [\n"), std::string::npos);
}

TEST_F(FormatTest, StringEscaping) {
    value obj;
    obj["msg"] = "Hello\nWorld\t\"Quote\"";
    std::string result = obj.to_str();
    
    EXPECT_NE(result.find("\\n"), std::string::npos);
    EXPECT_NE(result.find("\\t"), std::string::npos);
    EXPECT_NE(result.find("\\\""), std::string::npos);
}
TEST_F(FormatTest, FormatTypeEnum) {
    // Test format_type enum usage
    format fmt1(format_type::compact);
    EXPECT_EQ(fmt1.type, format_type::compact);
    
    format fmt2(format_type::pretty);
    EXPECT_EQ(fmt2.type, format_type::pretty);
    
    // Test assignment
    fmt1.type = format_type::pretty;
    EXPECT_EQ(fmt1.type, format_type::pretty);
}

TEST_F(FormatTest, FormatStaticMethods) {
    // Test format::get with different inputs
    format pretty = format::get("pretty");
    EXPECT_EQ(pretty.type, format_type::pretty);
    EXPECT_FALSE(pretty.key_no_quotes);
    
    format compact = format::get("compact");
    EXPECT_EQ(compact.type, format_type::compact);
    EXPECT_FALSE(compact.key_no_quotes);
    
    format xpretty = format::get("xpretty");
    EXPECT_EQ(xpretty.type, format_type::pretty);
    EXPECT_TRUE(xpretty.key_no_quotes);
    
    format xcompact = format::get("xcompact");
    EXPECT_EQ(xcompact.type, format_type::compact);
    EXPECT_TRUE(xcompact.key_no_quotes);
    
    // Invalid type
    EXPECT_THROW(format::get("PRETTY"), std::invalid_argument);
}

TEST_F(FormatTest, ComplexFormatting) {
    // Create complex nested structure
    value complex;
    complex["metadata"]["version"] = "1.0";
    complex["metadata"]["author"] = "test";
    complex["data"]["items"].append("item1");
    complex["data"]["items"].append("item2");
    complex["data"]["count"] = 2;
    complex["flags"]["enabled"] = true;
    complex["flags"]["debug"] = false;
    
    // Test with different format options
    format fmt(format_type::pretty);
    fmt.indent = 2;
    std::string result = fmt.to_str();
    EXPECT_FALSE(result.empty());
    
    // Test compact format
    format compact_fmt(format_type::compact);
    result = compact_fmt.to_str();
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.find('\n'), std::string::npos);
}