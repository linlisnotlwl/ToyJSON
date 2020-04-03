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
            fprintf(stderr, "ERROR:@%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

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

static void test_parse_expect_value() {
    Toy::JsonVar v;

    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::EXPECT_VALUE, Toy::Json::parse("", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());

    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::EXPECT_VALUE, Toy::Json::parse(" ", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_parse_invalid_value() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::INVALID_VALUE, Toy::Json::parse("nul", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());

    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::INVALID_VALUE, Toy::Json::parse("?", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_parse_root_not_singular() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::ROOT_NOT_SINGULAR, Toy::Json::parse("null x", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}
int main()
{
    
    test_parse();
    printf("TEST RESULT:%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}