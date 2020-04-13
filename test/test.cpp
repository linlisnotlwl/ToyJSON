#include <stdio.h>
#include <string.h>
#include "../Json.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "ERROR:@ %s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

static void test_parse_null() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::OK, Toy::Json::parse("null", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_parse_true() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::OK, Toy::Json::parse("true", &v));
    EXPECT_EQ_INT(Toy::JsonVar::TRUE, v.getType());
}

static void test_parse_false() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::TRUE);
    EXPECT_EQ_INT(Toy::Json::OK, Toy::Json::parse("false", &v));
    EXPECT_EQ_INT(Toy::JsonVar::FALSE, v.getType());
}



#define TEST_NUMBER(expect, json)\
    do {\
        Toy::JsonVar v;\
        EXPECT_EQ_INT(Toy::Json::OK, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::NUMBER, v.getType());\
        EXPECT_EQ_DOUBLE(expect, v.getNumberVal());\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        Toy::JsonVar v;\
        v.setType(Toy::JsonVar::STRING);\
        EXPECT_EQ_INT(Toy::Json::OK, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::STRING, v.getType());\
        EXPECT_EQ_STRING(expect, v.getCStr(), v.getCStrLength());\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");

#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}

#define TEST_ERROR(error, json)\
    do {\
        Toy::JsonVar v;\
        v.setType(Toy::JsonVar::FALSE);\
        EXPECT_EQ_INT(error, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());\
    } while(0)

static void test_parse_expect_value() {
    TEST_ERROR(Toy::Json::EXPECT_VALUE, "");
    TEST_ERROR(Toy::Json::EXPECT_VALUE, " ");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(Toy::Json::ROOT_NOT_SINGULAR, "null x");
    /* invalid number */
    /* after zero should be '.' or nothing */
    TEST_ERROR(Toy::Json::ROOT_NOT_SINGULAR, "0123"); 
    TEST_ERROR(Toy::Json::ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(Toy::Json::ROOT_NOT_SINGULAR, "0x123");
    TEST_ERROR(Toy::Json::ROOT_NOT_SINGULAR, "1.8b");
}

static void test_parse_number_too_big() {
    TEST_ERROR(Toy::Json::NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(Toy::Json::NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_invalid_value() {
    TEST_ERROR(Toy::Json::INVALID_VALUE, "nul");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "?");
    /* ... */
    /* invalid number */
    TEST_ERROR(Toy::Json::INVALID_VALUE, "+0");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "+1");
    TEST_ERROR(Toy::Json::INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(Toy::Json::INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(Toy::Json::INVALID_VALUE, "INF");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "inf");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "NAN");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "nan");

    TEST_ERROR(Toy::Json::INVALID_VALUE, "-.1");
    TEST_ERROR(Toy::Json::INVALID_VALUE, "e-100");
}
static void test_parse_missing_quotation_mark() {
    TEST_ERROR(Toy::Json::MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(Toy::Json::MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {

#if 1
    TEST_ERROR(Toy::Json::INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(Toy::Json::INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(Toy::Json::INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(Toy::Json::INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {

#if 1
    TEST_ERROR(Toy::Json::INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(Toy::Json::INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

static void test_access_null() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::NULL_TYPE);
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_access_boolean() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::FALSE);
    v.setBoolVal(false);
    EXPECT_FALSE(v.getBoolVal());
    v.setType(Toy::JsonVar::TRUE);
    v.setBoolVal(true);
    EXPECT_TRUE(v.getBoolVal());
}

static void test_access_number() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::NUMBER);
    v.setNumberVal(-1.0);
    TEST_NUMBER(v.getNumberVal(), "-1.0");
    v.setNumberVal(1.5);
    TEST_NUMBER(v.getNumberVal(), "1.5");
    v.setNumberVal(-1.5);
    TEST_NUMBER(v.getNumberVal(), "-1.5");
    v.setNumberVal(3.1416);
    TEST_NUMBER(v.getNumberVal(), "3.1416");
}

static void test_access_string() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("", 0);
    EXPECT_EQ_STRING("", v.getCStr(), v.getCStrLength());
    v.setCStr("Hello", 5);
    EXPECT_EQ_STRING("Hello", v.getCStr(), v.getCStrLength());
}
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_number();
    test_parse_invalid_value();
    test_parse_number_too_big();

    test_parse_string();
    test_parse_invalid_string_escape();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_char();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();

}
int main()
{
    
    test_parse();
    printf("TEST RESULT:%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}