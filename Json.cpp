#include <cassert>
#include <cstdlib>  // strtod() malloc
#include <cctype>   // for isalpha
#include <cerrno>   // for errno, ERANGE
#include <cmath>    // for HUGE_VAL
#include <cstring>  // for memcpy
#include "Json.h"

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
namespace Toy
{
// -----------------------------------------------xx implement start :
// -----------------------------------------------xx implement end !

// -----------------------------------------------JsonVar implement start :

double JsonVar::getNumberVal() const
{
    assert(m_type == JsonType::NUMBER);
    return m_val.num_val;
}

void JsonVar::setNumberVal(double val)
{
    assert(m_type == JsonType::NUMBER);
    //freeStrMem(); // no need: becasue of above 
    m_val.num_val = val;
}

bool JsonVar::getBoolVal() const
{
    assert(m_type == JsonType::FALSE || m_type == JsonType::TRUE);
    return m_val.bool_val;
}

void JsonVar::setBoolVal(bool b)
{
    assert(m_type == JsonType::FALSE || m_type == JsonType::TRUE);
    //freeStrMem(); // no need: becasue of above 
    m_val.bool_val = b;
}

const char *JsonVar::getCStr() const
{
    assert(m_type == JsonVar::STRING);
    return m_val.str.s;
}

void JsonVar::setCStr(const char * s, size_t len)
{
    
    assert(m_type == JsonVar::STRING); 
    freeStrMem();   
    m_val.str.s = static_cast<char *>(malloc(sizeof(char) * (len + 1)));
    memcpy(m_val.str.s, s, len);
    m_val.str.s[len] = '\0';
    m_val.str.len = len;
    // set str type ?? no, user should set it to JsonType::STRING before using this function
}

size_t JsonVar::getCStrLength() const
{
    assert(m_type == JsonVar::STRING);
    return m_val.str.len;
}

void JsonVar::freeStrMem()
{
    if(m_type == JsonType::STRING && m_val.str.s != nullptr)
    {
        free(m_val.str.s);
        m_val.str.s = nullptr;
        m_val.str.len = 0;
    }
}

size_t JsonVar::getArraySize() const
{
    //assert(m_type == JsonVar::ARRAY);
    return m_val.array.size;
}
void JsonVar::setArraySize(size_t size)
{
    assert(m_type == JsonVar::ARRAY);
    m_val.array.size = size;
}
void JsonVar::setArray(JsonVar *jv, size_t size)
{
    assert(m_type == JsonVar::ARRAY);
    m_val.array.jv = jv;
    m_val.array.size = size;
}
// -----------------------------------------------JsonVar implement end !
// -----------------------------------------------Context implement start :

// -----------------------------------------------Context implement end !
// -----------------------------------------------Json implement start :
Json::ParseStatus Json::parse(const char *json_text, JsonVar *out_jv)
{
    Context c;
    Json::ParseStatus jps;
    c.json = json_text;
    out_jv->setType(JsonVar::JsonType::NULL_TYPE);
    parseWhitespace(&c);
    if((jps = parseValue(&c, out_jv)) == Json::OK)
    {
        parseWhitespace(&c);
        if(*c.json != '\0') 
        {
            // TODO : is it necessary?
            out_jv->setType(JsonVar::NULL_TYPE);
            return Json::ROOT_NOT_SINGULAR;
        }
            
    }
    return jps;
}

void Json::parseWhitespace(Context *c)
{
    const char * p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        ++p;
    c->json = p;
}

Json::ParseStatus Json::parseValue(Context *c, JsonVar * out_jv)
{
    switch (*c->json)
    {
    case 'n' : return parseNull(c, out_jv);
    case '\0' : return Json::EXPECT_VALUE;
    case 'f' : return parseFalse(c, out_jv);
    case 't' : return parseTrue(c, out_jv);
    case '\"' : return parseCStr(c, out_jv);
    case '[' : return parseArray(c, out_jv);
    default : return parseNumber(c, out_jv);
    }
}

Json::ParseStatus Json::parseNull(Context *c, JsonVar * out_jv)
{
    assert( *c->json == 'n' );
    if(c->json[1] == 'u' && c->json[2] == 'l' && c->json[3] == 'l')
    {
        out_jv->setType(JsonVar::JsonType::NULL_TYPE);
        c->json += 4;
        return Json::OK;
    }
    else
        return Json::INVALID_VALUE;
}

Json::ParseStatus Json::parseFalse(Context *c, JsonVar * out_jv)
{
    assert( *c->json == 'f' );
    if(c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e')
    {
        out_jv->setType(JsonVar::JsonType::FALSE);
        c->json += 5;
        return Json::OK;
    }
    else
        return Json::INVALID_VALUE;
}

Json::ParseStatus Json::parseTrue(Context *c, JsonVar * out_jv)
{
    assert( *c->json == 't' );
    if(c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e')
    {
        out_jv->setType(JsonVar::JsonType::TRUE);
        c->json += 4;
        return Json::OK;
    }
    else
        return Json::INVALID_VALUE;
}

Json::ParseStatus Json::parseNumber(Context * c, JsonVar * out_jv)
{
    char *end = nullptr;
    // if we check number using index, 
    // it will make mistake or miss some error case. Like: -.1
    // if(c->json[0] == '+' || c->json[0] == '.' || isalpha(c->json[0]))
    //     return Json::INVALID_VALUE;
    // if(c->json[0] == '0' && c->json[1] != '.' && c->json[1] != '\0')
    //     return Json::ROOT_NOT_SINGULAR;

    // number = [ "-" ] int [ frac ] [ exp ]
    // int = "0" / digit1-9 *digit
    // frac = "." 1*digit
    // exp = ("e" / "E") ["-" / "+"] 1*digit
    const char * p = c->json;
    // check '-'
    if(*p == '-') 
        p++;
    // check '0'
    if(*p == '0')
        p++;
    else    // check behind the '-': should be number
    {
        // here will check invaild '.' or other
        if(!ISDIGIT1TO9(*p)) return Json::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p); // jump pass the number
    }
    /* after zero should be '.' or nothing */
    if(*p == '.')
    {
        p++;
        if(!ISDIGIT(*p)) return Json::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);
    }
    
    // check 'e' or 'E'
    if(*p == 'e' || *p == 'E')
    {
        p++;
        if(*p == '-' || *p == '+') p++; // 1e10 is correct
        if(!ISDIGIT(*p)) return Json::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);

    }

    // prase() with check the behind of *p
    // to see if there something left.
    // Like : 0123 (parse 0, left 123) 
    
    errno = 0;  // c-type error handling
    double temp = strtod(c->json, &end);
    //ERANGE:Result too large
    //HUGE_VAL:positive double expression that indicates overflow
    if(errno == ERANGE && (temp == HUGE_VAL || temp == -HUGE_VAL)) 
        return Json::NUMBER_TOO_BIG;

    c->json = p;
    out_jv->setType(JsonVar::NUMBER);
    out_jv->setNumberVal(temp);
    return Json::OK;
}

Json::ParseStatus HandleAndReturnError(Context * c, JsonVar * out_jv, Json::ParseStatus status)
{
    c->parse_stack.clear();
    // TODO : is it necessary?
    out_jv->setType(JsonVar::NULL_TYPE);
    return status;
}
Json::ParseStatus Json::parseCStr(Context * c, JsonVar * out_jv)
{
    const char * p = c->json;
    assert(*p == '\"');
    u_int32_t u; // save utf-8 code
    for(;;)
    {
        p++;
        switch(*p)
        {
            case '\"' : 
            {  
                out_jv->setType(JsonVar::STRING);
                out_jv->setCStr(reinterpret_cast<char *>(c->parse_stack.getData()), c->parse_stack.getSize());
                c->parse_stack.clear();
                c->json = p + 1;
                return Json::ParseStatus::OK;
            }
            case '\0' :
                return HandleAndReturnError(c, out_jv, Json::ParseStatus::MISS_QUOTATION_MARK);
            case '\\' :
            {
                p++;
                switch(*p)
                {
                    case 'n' : *(c->parse_stack.push<char>()) = '\n'; break;
                    case '/' : *(c->parse_stack.push<char>()) = '/'; break;
                    case 'b' : *(c->parse_stack.push<char>()) ='\b'; break;
                    case 'f' : *(c->parse_stack.push<char>()) ='\f'; break;
                    case 'r' : *(c->parse_stack.push<char>()) ='\r'; break;
                    case 't' : *(c->parse_stack.push<char>()) ='\t'; break;
                    case '\\' : *(c->parse_stack.push<char>()) ='\\'; break;
                    case '\"' : *(c->parse_stack.push<char>()) ='\"'; break;
                    case 'u' :
                    {
                        p++;
                        if(!parseHex4(p, u))
                            return HandleAndReturnError(c, out_jv, Json::INVALID_UNICODE_HEX);
                        
                        // check UNICODE_SURROGATE
                        if(u >= 0xDC00)// low surrogate occur with a high surrogate
                            return HandleAndReturnError(c, out_jv, Json::INVALID_UNICODE_SURROGATE);
                        if(u >= 0xD800 && u <= 0xDBFF) // high surrogate
                        {
                            p += 4; // jump to next code. should begin with '\\'
                            if(*p != '\\' || *(p+1) != 'u')
                                return HandleAndReturnError(c, out_jv, Json::INVALID_UNICODE_SURROGATE);
                            u_int32_t low_s;
                            if(!parseHex4(p+2, low_s))
                                return HandleAndReturnError(c, out_jv, Json::INVALID_UNICODE_SURROGATE);
                            if(low_s < 0xDC00 || low_s > 0xDFFF)
                                return HandleAndReturnError(c, out_jv, Json::INVALID_UNICODE_SURROGATE);
                            
                            u = 0x10000 + ((u - 0xD800) << 10) + (low_s - 0xDC00);

                            p += 2; // jump to first X of \\uXXXX
                        }

                        assert(u >= 0 && u <= 0x10FFFF);
                        if (u >= 0 && u <= 0x007F)
                            *(c->parse_stack.push<char>()) = u;
                        else if (u >= 0x0080 && u <= 0x07FF)
                        {
                            // byte1 : 110x xxxx. 0xC0: 1100 0000.  0x1F:    1 1111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            *(c->parse_stack.push<char>()) = 0xC0 | ((u >> 6) & 0x1F);
                            *(c->parse_stack.push<char>()) =0x80 | (u & 0x3F);
                        }
                        else if (u >= 0x0800 && u <= 0xFFFF)
                        {
                            // byte1 : 1110 xxxx. 0xE0: 1110 0000.  0x0F:      1111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            // byte3 like byte2;
                            *(c->parse_stack.push<char>()) = 0xE0 | ((u >> 12) & 0x0F);
                            *(c->parse_stack.push<char>()) = 0x80 | ((u >> 6) & 0x3F);
                            *(c->parse_stack.push<char>()) = 0x80 | (u & 0x3F);
                        }
                        else
                        {
                            // byte1 : 1111 0xxx. 0xF0: 1111 0000.  0x08:      0111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            // byte3 byte4 like byte2;
                            *(c->parse_stack.push<char>()) = 0xF0 | ((u >> 18) & 0x08);
                            *(c->parse_stack.push<char>()) = 0x80 | ((u >> 12) & 0x3F);
                            *(c->parse_stack.push<char>()) = 0x80 | ((u >> 6) & 0x3F);
                            *(c->parse_stack.push<char>()) = 0x80 | (u & 0x3F);
                        }
                        p += 3; // p point at 'u', so jump to last number
                        break;
                    }
                    default : 
                        return HandleAndReturnError(c, out_jv, Json::INVALID_STRING_ESCAPE);   
                }
                break;  
            }
            default : 
            {
                if(*p >= '\x00' && *p <= '\x1F')
                {
                    return HandleAndReturnError(c, out_jv, Json::INVALID_STRING_CHAR);
                }
                else
                    *(c->parse_stack.push<char>()) = *p;  
            }
        }
    }
    // 优化思考：
    // 来源（https://github.com/miloyip/json-tutorial/blob/master/tutorial03_answer）
    // 1、假如没有转义字符，则不用进行转义判断，也不需要利用stack存储临时解析的char
    //    可以直接放入JsonVar中，从而减少一次拷贝。
    // 2、对于扫描没转义部分，我们可考虑用 SIMD 加速，
    //    如 RapidJSON 代码剖析（二）：使用 SSE4.2 优化字符串扫描 的做法。
    //  （https://zhuanlan.zhihu.com/p/20037058）
    //   这类底层优化的缺点是不跨平台，需要设置编译选项等。
    // 3、在 gcc/clang 上使用 __builtin_expect() 指令来处理低概率事件，
    //   例如需要对每个字符做 LEPT_PARSE_INVALID_STRING_CHAR 检测，
    //   我们可以假设出现不合法字符是低概率事件，然后用这个指令告之编译器，
    //   那么编译器可能可生成较快的代码。然而，这类做法明显是不跨编译器，
    //   甚至是某个版本后的 gcc 才支持
}
bool Json::parseHex4(const char * str, u_int32_t & u)
{
    int count = 0;
    u = 0;
    while(*str != '\0' && count < 4)
    {
        if(isxdigit(static_cast<unsigned char>(*str)) == 0) // or use std::isxdigit in <locale>
            return false;
        //u = (u << 4) | (charToUint32(*str) & 0x0F);
        u = (u << 4) | charToUint32(*str);
        count++;
        str++;
    }
    return count == 4;
    
}

u_int32_t Json::charToUint32(const char & c)
{
    assert(isxdigit(static_cast<unsigned char>(c)) != 0);
    if(isdigit(static_cast<unsigned char>(c)) != 0)
        return c - '0';
    switch(c)
    {
        case 'a' :
        case 'A' : return 10;
        case 'b' :
        case 'B' : return 11;
        case 'c' :
        case 'C' : return 12;
        case 'd' :
        case 'D' : return 13;
        case 'e' :
        case 'E' : return 14;
        //case 'f' :
        //case 'F' : return 15;
        default : return 15;
    }
}

Json::ParseStatus Json::parseArray(Context * c, JsonVar * out_jv)
{
    const char * p = c->json;
    assert(*p == '[');
    p++;
    if(*p == ']')
    {
        c->json = p + 1;
        out_jv->setType(JsonVar::ARRAY);
        out_jv->setArray(nullptr, 0);
        return Json::OK;
    }
    // for(;;)
    // {
        
    // }
}
// -----------------------------------------------Json implement end !
} // namespace Toy