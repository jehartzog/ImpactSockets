/**
 * Created by TekuConcept on July 30, 2018
 */

#include <vector>
#include <string>

#include <gtest/gtest.h>
#include <rfc/utf8.h>

#include <iostream>
#define VERB(x) std::cout << x << std::endl

namespace impact {
    class test_utf8_c {
    public:
        static size_t estimate_buf_size(char32_t __symbol) {
            return utf8::_S_estimate_buf_size(__symbol);
        }
        static void encode(const char32_t& __symbol, std::string* __str) {
            utf8::_S_encode(__symbol, __str);
        }
    };
}

using namespace impact;

TEST(test_utf8, size_estimate_char) {
    using test = test_utf8_c;
    EXPECT_EQ(test::estimate_buf_size(0x00), 1);
    EXPECT_EQ(test::estimate_buf_size('\0'), 1);
    EXPECT_EQ(test::estimate_buf_size('\x7F'), 1);
    // NOTE: '\xFF' requires a 32-bit mask
    EXPECT_EQ(test::estimate_buf_size(u'\u00FF'), 2);
    EXPECT_EQ(test::estimate_buf_size(u'\u07FF'), 2);
    EXPECT_EQ(test::estimate_buf_size(u'\uFFFF'), 3);
    EXPECT_EQ(test::estimate_buf_size(U'\U0010FFFF'), 4);
    EXPECT_EQ(test::estimate_buf_size(0x00110000), -1);
    
    /*
    0xD800 - 0xDFFF are reserved unicode values
    u'uD800' and U'U0000D800' are not allowed during compile-time
    but 0xD800 and 0x0000D800 values are allowed during both
    compile-time and runtime. Make sure the functions take this
    into account.
    */
    char16_t c4 = 0xDFFF;
    EXPECT_EQ(test::estimate_buf_size(c4), -1);
}

TEST(test_utf8, encode) {
    using test = test_utf8_c;
    std::string a, b;
    char32_t c;
    
    a.assign("");
    b.assign("\x00", 1);
    c = 0x00000000;
    test::encode(c, &a);
    EXPECT_EQ(a, b);
    
    a.assign("");
    b.assign("\xDF\xBF", 2);
    c = 0x000007FF;
    test::encode(c, &a);
    EXPECT_EQ(a, b);
    
    a.assign("");
    b.assign("\xEF\xBF\xBF", 3);
    c = 0x0000FFFF;
    test::encode(c, &a);
    EXPECT_EQ(a, b);
    
    a.assign("");
    b.assign("\xF4\x8F\xBF\xBF", 4);
    c = 0x0010FFFF;
    test::encode(c, &a);
    EXPECT_EQ(a, b);
}

TEST(test_utf8, serialize) {
    char32_t c;
    std::string result;
    std::string expected;
    
    c = 0x00000000;
    result.assign("");
    expected.assign("\x00", 1);
    EXPECT_TRUE(utf8::serialize(c, &result));
    EXPECT_EQ(result, expected);
    
    c = 0x0000007F;
    result.assign("");
    expected.assign("\x7F", 1);
    EXPECT_TRUE(utf8::serialize(c, &result));
    EXPECT_EQ(result, expected);
    
    c = 0x000007FF;
    result.assign("");
    expected.assign("\xDF\xBF", 2);
    EXPECT_TRUE(utf8::serialize(c, &result));
    EXPECT_EQ(result, expected);
    
    c = 0x0000FFFF;
    result.assign("");
    expected.assign("\xEF\xBF\xBF", 3);
    EXPECT_TRUE(utf8::serialize(c, &result));
    EXPECT_EQ(result, expected);
    
    c = 0x0010FFFF;
    result.assign("");
    expected.assign("\xF4\x8F\xBF\xBF", 4);
    EXPECT_TRUE(utf8::serialize(c, &result));
    EXPECT_EQ(result, expected);
    
    c = 0xDFFF; // make illegal
    EXPECT_FALSE(utf8::serialize(c, &result));
}

TEST(test_utf8, serialize1) {
    std::string input("\x00\x7F\xFF", 3);
    std::string expected("\x00\x7F\xC3\xBF", 4);
    
    std::string result;
    EXPECT_TRUE(utf8::serialize(input, &result));
    EXPECT_EQ(result, expected);
}

TEST(test_utf8, serialize2) {
    std::u16string input(
        u"\u0000"
        u"\u007F"
        u"\u07FF"
        u"\uFFFF", 4);
    std::string expected(
        "\x00"
        "\x7F"
        "\xDF\xBF"
        "\xEF\xBF\xBF", 7);
    
    std::string result;
    EXPECT_TRUE(utf8::serialize(input, &result));
    EXPECT_EQ(result, expected);
    
    input[0] = 0xDFFF; // make illegal
    EXPECT_FALSE(utf8::serialize(input, &result));
}

TEST(test_utf8, serialize3) {
    std::u32string input(
        U"\U00000000"
        U"\U0000007F"
        U"\U000007FF"
        U"\U0000FFFF"
        U"\U0010FFFF", 5);
    std::string expected(
        "\x00"
        "\x7F"
        "\xDF\xBF"
        "\xEF\xBF\xBF"
        "\xF4\x8F\xBF\xBF", 11);
    
    std::string result;
    EXPECT_TRUE(utf8::serialize(input, &result));
    EXPECT_EQ(result, expected);
    
    input[0] = 0x110000; // make illegal
    EXPECT_FALSE(utf8::serialize(input, &result));
}

TEST(test_utf8, serialize_empty) {
    char32_t c;
    
    c = 0x00000000;
    EXPECT_TRUE(utf8::serialize(c, NULL));
    c = 0x0000DFFF;
    EXPECT_FALSE(utf8::serialize(c, NULL));
    
    std::string str1("\0", 1);
    EXPECT_TRUE(utf8::serialize(str1, NULL));
    
    std::u16string str2(u"\u0000", 1);
    EXPECT_TRUE(utf8::serialize(str2, NULL));
    str2[0] = 0xDFFF;
    EXPECT_FALSE(utf8::serialize(str2, NULL));
    
    std::u32string str3(U"\U00000000", 1);
    EXPECT_TRUE(utf8::serialize(str3, NULL));
    str3[0] = 0x00110000;
    EXPECT_FALSE(utf8::serialize(str3, NULL));
}

TEST(test_utf8, serialize_append) {
    std::string result = "test";
    std::string expected("test");
    
    char32_t input1 = U'\U0010FFFF';
    expected.append("\xF4\x8F\xBF\xBF", 4);
    EXPECT_TRUE(utf8::serialize(input1, &result));
    ASSERT_EQ(result, expected);
    
    std::string input2("\x00\xFF", 2);
    expected.append("\x00\xC3\xBF", 3);
    EXPECT_TRUE(utf8::serialize(input2, &result));
    ASSERT_EQ(result, expected);
    
    std::u16string input3(u"\u0000\uFFFF", 2);
    expected.append("\x00\xEF\xBF\xBF", 4);
    EXPECT_TRUE(utf8::serialize(input3, &result));
    ASSERT_EQ(result, expected);
    
    std::u32string input4(U"\U00000000\U0010FFFF", 2);
    expected.append("\x00\xF4\x8F\xBF\xBF", 5);
    EXPECT_TRUE(utf8::serialize(input4, &result));
    ASSERT_EQ(result, expected);
}

TEST(test_utf8, deserialize) {
    std::string input(
        "\x00"
        "\x7F"
        "\xDF\xBF"
        "\xEF\xBF\xBF"
        "\xF4\x8F\xBF\xBF", 11);
    std::u32string expected(
        U"\U00000000"
        U"\U0000007F"
        U"\U000007FF"
        U"\U0000FFFF"
        U"\U0010FFFF", 5);
    
    std::u32string result;
    EXPECT_TRUE(utf8::deserialize(input, &result));
    EXPECT_EQ(expected, result);
    
    result.assign(U"");
    input[0] = '\xFF';
    EXPECT_FALSE(utf8::deserialize(input, &result));
}

TEST(test_utf8, deserialize_empty) {
    std::string input(
        "\x00"
        "\x7F"
        "\xDF\xBF"
        "\xEF\xBF\xBF"
        "\xF4\x8F\xBF\xBF", 11);
    
    EXPECT_TRUE(utf8::deserialize(input, NULL));
    
    input[0] = '\xFF';
    EXPECT_FALSE(utf8::deserialize(input, NULL));
}

TEST(test_utf8, deserialize_append) {
    std::string input("\x00\x7F\xDF\xBF", 4);
    std::u32string expected(U"\U00000000\U0000007F\U000007FF", 3);
    std::u32string result;
    
    EXPECT_TRUE(utf8::deserialize(input, &result));
    EXPECT_EQ(result, expected);
    
    input.assign("\xEF\xBF\xBF\xF4\x8F\xBF\xBF", 7);
    expected.append(U"\U0000FFFF\U0010FFFF", 2);
    EXPECT_TRUE(utf8::deserialize(input, &result));
    EXPECT_EQ(result, expected);
}
