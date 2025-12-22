#include <gtest/gtest.h>
#include "json/json.h"

using namespace sid::json;

class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ValueTest, DefaultConstructor) {
    value v;
    EXPECT_TRUE(v.is_null());
    EXPECT_EQ(v.type(), value_type::null);
}

TEST_F(ValueTest, BooleanOperations) {
    value v(true);
    EXPECT_TRUE(v.is_bool());
    EXPECT_EQ(v.get_bool(), true);
    
    v = false;
    EXPECT_EQ(v.get_bool(), false);
}

TEST_F(ValueTest, NumericOperations) {
    // Signed integer
    value v(42);
    EXPECT_TRUE(v.is_num());
    EXPECT_TRUE(v.is_signed());
    EXPECT_EQ(v.get_int64(), 42);
    
    // Unsigned integer
    v = static_cast<uint64_t>(100);
    EXPECT_TRUE(v.is_unsigned());
    EXPECT_EQ(v.get_uint64(), 100);
    
    // Double
    v = 3.14;
    EXPECT_TRUE(v.is_double());
    EXPECT_DOUBLE_EQ(v.get_double(), 3.14);
}

TEST_F(ValueTest, StringOperations) {
    value v("hello");
    EXPECT_TRUE(v.is_string());
    EXPECT_EQ(v.get_str(), "hello");
    
    v = std::string("world");
    EXPECT_EQ(v.get_str(), "world");
}

TEST_F(ValueTest, ArrayOperations) {
    value arr;
    // Array is created by calling append
    arr.append(1);
    arr.append("test");
    arr.append(true);
    
    EXPECT_TRUE(arr.is_array());
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get_int64(), 1);
    EXPECT_EQ(arr[1].get_str(), "test");
    EXPECT_EQ(arr[2].get_bool(), true);
    
    // Test erase by index
    arr.erase(1); // Remove "test"
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].get_int64(), 1);
    EXPECT_EQ(arr[1].get_bool(), true);
    
    // Test erase first element
    arr.erase(0);
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr[0].get_bool(), true);
}

TEST_F(ValueTest, ObjectOperations) {
    value obj;
    // Object is created by using [] operator
    obj["name"] = "John";
    obj["age"] = 30;
    obj["active"] = true;
    
    EXPECT_TRUE(obj.is_object());
    EXPECT_EQ(obj.size(), 3);
    EXPECT_TRUE(obj.has_key("name"));
    EXPECT_EQ(obj["name"].get_str(), "John");
    EXPECT_EQ(obj["age"].get_int64(), 30);
    EXPECT_EQ(obj["active"].get_bool(), true);
}

TEST_F(ValueTest, TypeConversions) {
    value v;
    
    // Test type changes
    v = 42;
    EXPECT_TRUE(v.is_signed());
    
    v = "string";
    EXPECT_TRUE(v.is_string());
    
    v = true;
    EXPECT_TRUE(v.is_bool());
}

TEST_F(ValueTest, CopyAndAssignment) {
    value original;
    original["key"] = "value";
    
    value copy = original;
    EXPECT_EQ(copy["key"].get_str(), "value");
    
    value assigned;
    assigned = original;
    EXPECT_EQ(assigned["key"].get_str(), "value");
}

TEST_F(ValueTest, EraseOperations) {
    // Test array erase operations
    value arr;
    arr.append(10);
    arr.append(20);
    arr.append(30);
    arr.append(40);
    
    EXPECT_EQ(arr.size(), 4);
    
    // Erase middle element
    arr.erase(2); // Remove 30
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get_int64(), 10);
    EXPECT_EQ(arr[1].get_int64(), 20);
    EXPECT_EQ(arr[2].get_int64(), 40);
    
    // Erase last element
    arr.erase(2); // Remove 40
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].get_int64(), 10);
    EXPECT_EQ(arr[1].get_int64(), 20);

    // Erase out-of-range element (should throw)
    EXPECT_THROW(arr.erase(50), std::out_of_range);
    
    // Test object erase operations
    value obj;
    obj["key1"] = "value1";
    obj["key2"] = "value2";
    obj["key3"] = "value3";
    
    EXPECT_EQ(obj.size(), 3);
    EXPECT_TRUE(obj.has_key("key2"));
    
    // Erase key
    obj.erase("key2");
    EXPECT_EQ(obj.size(), 2);
    EXPECT_FALSE(obj.has_key("key2"));
    EXPECT_TRUE(obj.has_key("key1"));
    EXPECT_TRUE(obj.has_key("key3"));
}

TEST_F(ValueTest, EraseErrorHandling) {
    value arr;
    arr.append(1);
    arr.append(2);
    
    // Test out of bounds erase
    EXPECT_THROW(arr.erase(5), std::out_of_range);
    
    // Test erase on non-array
    value obj;
    obj["key"] = "value";
    EXPECT_THROW(obj.erase(0), std::runtime_error);
    
    // Test erase non-existent key (should not throw)
    obj.erase("nonexistent");
    EXPECT_EQ(obj.size(), 1);
}