#pragma once


namespace Toy
{
class JsonVar
{
public:
    enum JsonType
    {
        NULL_TYPE = 0,
        FALSE,
        TRUE,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };
    JsonType getType() const { return m_type; }
    void setType(const JsonType & type) { m_type = type; }
private:
    JsonType m_type;

};
struct Context
{
    const char * json;
};

class Json
{
public:
    enum ParseStatus
    {
        OK = 0,             /// 正常
        EXPECT_VALUE,       /// 只含有空白
        INVALID_VALUE,      /// 值不是所定义的字面值
        ROOT_NOT_SINGULAR   /// 一个值之后，在空白之后还有其他字符
    };
    static ParseStatus parse(const char *json_text, JsonVar * out_jv);
private:
    static void parseWhitespace(Context * c);
    static ParseStatus parseValue(Context * c, JsonVar * out_jv);
    static ParseStatus parseNull(Context * c, JsonVar * out_jv);
    static ParseStatus parseFalse(Context * c, JsonVar * out_jv);
    static ParseStatus parseTrue(Context * c, JsonVar * out_jv);
};

} // namesapce Toy
