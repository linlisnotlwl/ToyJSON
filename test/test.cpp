#include <stdio.h>
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
}

//TODO
// static void test_parse_number_too_big() {
//     TEST_ERROR(, "");
//     TEST_ERROR(, "");
// }

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
}
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_number();
    test_parse_invalid_value();
}
int main()
{
    
    test_parse();
    printf("TEST RESULT:%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}