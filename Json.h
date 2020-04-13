#pragma once

#include <vector>



namespace Toy
{
class JsonVar
{
public:
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
    JsonVar(){ m_val.str.s = nullptr; m_val.str.len = 0; }
    ~JsonVar() { freeStrMem(); }
    JsonType getType() const {  return m_type; }
    void setType(const JsonType & type) { freeStrMem(); m_type = type; }
    double getNumberVal() const;
    void setNumberVal(double);
    bool getBoolVal() const;
    void setBoolVal(bool);
    const char * getCStr() const;
    void setCStr(const char *, size_t);
    size_t getCStrLength() const;
private:
    void freeStrMem();
    JsonType m_type = JsonType::NULL_TYPE;  // init to be null type
    union Val
    {
        bool bool_val;
        double num_val;
        struct {
            char * s;
            size_t len;
        } str;
    } m_val;

};

// /// TODO : reconstruct the code
// class NullVar : public JsonVar
// {
// public:   
// private:
// };


struct Context
{
    const char * json;
    /// container : store the temp parsed char
    std::vector<char> char_stack;
};

class Json
{
public:
    enum ParseStatus
    {
        OK = 0,                 /// 正常
        EXPECT_VALUE,           /// 只含有空白
        INVALID_VALUE,          /// 值不是所定义的字面值
        ROOT_NOT_SINGULAR,      /// 一个值之后，在空白之后还有其他字符
        NUMBER_TOO_BIG,         /// 数值太大
        MISS_QUOTATION_MARK,    /// 字符串缺少 " 
        INVALID_STRING_ESCAPE,  /// 无效的转义字符
        INVALID_STRING_CHAR     /// 无效的字符
    };
    static ParseStatus parse(const char *json_text, JsonVar * out_jv);
private:
    static void parseWhitespace(Context * c);
    static ParseStatus parseValue(Context * c, JsonVar * out_jv);
    static ParseStatus parseNull(Context * c, JsonVar * out_jv);
    static ParseStatus parseFalse(Context * c, JsonVar * out_jv);
    static ParseStatus parseTrue(Context * c, JsonVar * out_jv);
    static ParseStatus parseNumber(Context * c, JsonVar * out_jv);
    static ParseStatus parseCStr(Context * c, JsonVar * out_jv);
    static const char transEscape(const char &);
    
};

} // namesapce Toy
