#pragma once

//#include <vector>
#include <sys/types.h>  // for u_int32_t
#include <unordered_map>
#include <map>
#include <string>

#include "PureStack.h"

namespace Toy
{
class JsonVar
{
public:
    typedef std::multimap<std::string, JsonVar> Object;
    enum JsonType
    {
        NULL_TYPE = 0,  // 命名为NULL会冲突？？
        FALSE,
        TRUE,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };
    JsonVar();
    JsonVar(JsonVar && jv);
    ~JsonVar();
    JsonVar & operator=(JsonVar && jv);
    JsonType getType() const {  return m_type; }
    // FIXME : set to string but s is not null, free coredump
    void setType(const JsonType & type) { freeMem(); m_type = type; }
    // number 
    double getNumberVal() const;
    void setNumberVal(double);
    // boolean
    bool getBoolVal() const;
    void setBoolVal(bool);
    // str
    const char * getCStr() const;
    void setCStr(const char *, size_t);
    size_t getCStrLength() const;
    // array
    size_t getArraySize() const;
    void setArraySize(size_t size);
    void setArray(JsonVar *jv, size_t size);
    const JsonVar * getArrayElememt(size_t index) const;
    JsonVar * getArrayElememt(size_t index);
    // object
    void setObject(Object *);
    Object * getObject();
    const Object * getObject() const;
    size_t getObjectSize() const;
    JsonVar * getObjectValue(const std::string &);
private:
    void reset() { memset(&m_val, 0, sizeof(m_val)); }
    void freeMem();
    JsonType m_type = JsonType::NULL_TYPE;  // init to be null type
    union Val
    {
        bool bool_val;
        double num_val;
        struct 
        {
            size_t len;
            char * s;
        } str;
        struct 
        {
            size_t size;    // element num
            JsonVar *jv;    // array pointer, must destruct all elements  
        } array;
        
        Object * object_p;  
    } m_val;
};

/// TODO : reconstruct the code
// class NullVar : public JsonVar
// {
// public:   
// private:
// };


struct Context
{
    const char * json;
    /// container : store the temp element
    PureStack elem_stack;
};

class Json
{
public:
    enum ParseStatus
    {
        OK = 0,                         /// 0 : 正常
        EXPECT_VALUE,                   /// 1 : 只含有空白
        INVALID_VALUE,                  /// 2 : 值不是所定义的字面值
        ROOT_NOT_SINGULAR,              /// 3 : 一个值之后，在空白之后还有其他字符
        NUMBER_TOO_BIG,                 /// 4 : 数值太大
        MISS_QUOTATION_MARK,            /// 5 : 字符串缺少 " 
        INVALID_STRING_ESCAPE,          /// 6 : 无效的转义字符
        INVALID_STRING_CHAR,            /// 7 : 无效的字符
        INVALID_UNICODE_SURROGATE,      /// 8 : 不正确的代理对范围
        INVALID_UNICODE_HEX,            /// 9 : \u 后面不是4位十六进制数字
        MISS_COMMA_OR_SQUARE_BRACKET,   /// 10 : [ ] 数组括号缺失，不匹配
        MISS_KEY,                       /// 11 : 缺失 键值
        MISS_COLON,                     /// 12 : 缺失 : 
        MISS_COMMA_OR_CURLY_BRACKET     /// 13 : 缺失 }
    };
    enum StringifyStatus
    {
        SUCCESS
    };
    static ParseStatus parse(const char *json_text, JsonVar * out_jv);
    static std::string stringify(const JsonVar *in_jv);
private:
    static void parseWhitespace(Context * c);
    static ParseStatus parseValue(Context * c, JsonVar * out_jv);
    static ParseStatus parseNull(Context * c, JsonVar * out_jv);
    static ParseStatus parseFalse(Context * c, JsonVar * out_jv);
    static ParseStatus parseTrue(Context * c, JsonVar * out_jv);
    static ParseStatus parseNumber(Context * c, JsonVar * out_jv);
    static ParseStatus parseCStr(Context * c, JsonVar * out_jv);
    static bool parseHex4(const char *, u_int32_t &);
    static u_int32_t charToUint32(const char & c);
    static ParseStatus parseArray(Context * c, JsonVar * out_jv);
    static ParseStatus parseObject(Context * c, JsonVar * out_jv);

    static void stringifyValue(Context * c, const JsonVar *in_jv);
    static void stringifyLiteral(char * buf, const char * str, size_t size);
    static void stringifyString(Context * c, const char * str, size_t length);
    static void stringifyArray(Context * c, const JsonVar *in_jv);
    static void stringifyObject(Context * c, const JsonVar *in_jv);
};

} // namesapce Toy
