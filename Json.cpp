#include <cassert>
#include <cstdlib>  // strtod() malloc
#include <cctype>   // for isalpha
#include <cerrno>   // for errno, ERANGE
#include <cmath>    // for HUGE_VAL
#include <cstring>  // for memcpy
#include <exception>
#include <fstream> // for ifstream ofstream
#include <sstream>
#include "Json.h"

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

static constexpr int STRINGIFY_NUM_BUF_SIZE = 32;
namespace Toy
{
// -----------------------------------------------xx implement start :
// -----------------------------------------------xx implement end !

// -----------------------------------------------JsonVar implement start :

JsonVar::JsonVar(const JsonVar::JsonType & type) : m_type(type)
{
    reset();
}

JsonVar::JsonVar(const JsonVar &jv)
{
    m_type = jv.m_type;
    switch(jv.m_type)
    {
        case JsonVar::JsonType::NULL_TYPE : 
            break;
        case JsonVar::JsonType::FALSE : 
            m_val.bool_val = false;
            break;
        case JsonVar::JsonType::TRUE : 
            m_val.bool_val = true;
            break;
        case JsonVar::JsonType::NUMBER : 
            m_val.num_val = jv.m_val.num_val;
            break;
        case JsonVar::JsonType::STRING :
            m_val.str.s = static_cast<char *>(malloc(jv.m_val.str.len+1));
            memcpy(m_val.str.s, jv.m_val.str.s, jv.m_val.str.len + 1);
            m_val.str.len = jv.m_val.str.len;
            break;
        case JsonVar::JsonType::ARRAY : 
            if(jv.getArraySize() == 0)
                m_val.array_p = new JsonVar::Array;  // if we do not new , it will occur an error at test_parse_array(137)
            else
                m_val.array_p = new JsonVar::Array(jv.m_val.array_p->begin(),jv.m_val.array_p->end());
            break;
        case JsonVar::JsonType::OBJECT : 
            if(jv.getObjectSize() == 0)
                m_val.object_p = new JsonVar::Object;
            else
            m_val.object_p = new JsonVar::Object(jv.m_val.object_p->begin(), jv.m_val.object_p->end());
            break;
        default : // error type : should not happen
            throw std::exception();  
    }
}

JsonVar::JsonVar(JsonVar &&jv) 
{ 
    m_val = jv.m_val;
    m_type = jv.m_type;
    jv.m_type = JsonVar::NULL_TYPE;
    jv.reset();
}

JsonVar::~JsonVar() { freeMem(); }

const JsonVar & JsonVar::operator=(JsonVar && jv)
{
    m_val = jv.m_val;
    m_type = jv.m_type;
    jv.m_type = JsonVar::NULL_TYPE;
    jv.reset();
    return *this;
}

const JsonVar & JsonVar::operator=(const JsonVar & jv)
{
    freeMem();
    JsonVar temp(jv);
    *this = std::move(temp);
    return *this;
}

void swap(JsonVar &left, JsonVar &right)
{
    using std::swap;    // use std version if it has not its own swap.
    swap(left.m_type, right.m_type);
    swap(left.m_val, right.m_val);
}

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
    freeMem();   
    m_val.str.s = static_cast<char *>(malloc(sizeof(char) * (len + 1)));
    memcpy(m_val.str.s, s, len);
    m_val.str.s[len] = '\0'; // !!!
    m_val.str.len = len;
    // set str type ?? no, user should set it to JsonType::STRING before using this function
}

size_t JsonVar::getCStrLength() const
{
    assert(m_type == JsonVar::STRING);
    return m_val.str.len;
}

void JsonVar::freeMem()
{
    if(m_type == JsonType::STRING && m_val.str.s != nullptr)
    {
        free(m_val.str.s);
        m_val.str.s = nullptr;
        m_val.str.len = 0;
    }
    if(m_type == JsonType::ARRAY && m_val.array_p != nullptr)
    {
        delete m_val.array_p;
        m_val.array_p = nullptr;
    }
    if(m_type == JsonType::OBJECT && m_val.object_p != nullptr)
    {
        delete m_val.object_p;
        m_val.object_p = nullptr;
    }
}

size_t JsonVar::getArraySize() const
{
    assert(m_type == JsonVar::ARRAY);
    return m_val.array_p == nullptr ? 0 : m_val.array_p->size();
}

void JsonVar::setArray(JsonVar::Array *ap)
{
    assert(m_type == JsonVar::ARRAY);
    freeMem();
    m_val.array_p = ap;
}

JsonVar::Array * JsonVar::getArray()
{
    assert(m_type == JsonVar::ARRAY);
    return m_val.array_p;
}

const JsonVar::Array * JsonVar::getArray() const
{
    assert(m_type == JsonVar::ARRAY);
    return m_val.array_p;
}

const JsonVar * JsonVar::getArrayElememt(size_t index) const
{
    assert(m_type == JsonVar::ARRAY && getArraySize() > index);
    return &(*m_val.array_p)[index];
}

JsonVar * JsonVar::getArrayElememt(size_t index)
{
    assert(m_type == JsonVar::ARRAY && getArraySize() > index);
    return &(*m_val.array_p)[index];
}

void JsonVar::addArrayElement(const JsonVar & jv)
{
    assert(m_type == JsonVar::ARRAY);
    if(m_val.array_p == nullptr)
        m_val.array_p = new JsonVar::Array;
    m_val.array_p->push_back(jv); 
}

void JsonVar::addArrayElement(JsonVar && jv)
{
    assert(m_type == JsonVar::ARRAY);
    if(m_val.array_p == nullptr)
        m_val.array_p = new JsonVar::Array;
    m_val.array_p->push_back(jv); 
}

void JsonVar::setObject(Object * op)
{
    assert(m_type == JsonVar::OBJECT);
    freeMem();
    m_val.object_p = op;
}

JsonVar::Object * JsonVar::getObject()
{
    return m_val.object_p;
}

const JsonVar::Object * JsonVar::getObject() const
{
    return m_val.object_p;
}

size_t JsonVar::getObjectSize() const
{
    assert(m_type == JsonVar::OBJECT);
    return m_val.object_p == nullptr ? 0 : m_val.object_p->size();
}

JsonVar * JsonVar::getObjectValue(const std::string & key)
{
    assert(m_type == JsonVar::OBJECT);
    if(m_val.object_p != nullptr)
    {
        auto it = m_val.object_p->find(key);
        if(it != m_val.object_p->end())
            return &(it->second);
    }
    return nullptr;
}

void JsonVar::addObjectElement(const std::string & key, const JsonVar & value)
{
    assert(m_type == JsonVar::OBJECT);
    if(m_val.object_p == nullptr)
        m_val.object_p = new JsonVar::Object;
    m_val.object_p->insert(std::make_pair(key, value)); // copy assignment
}

void JsonVar::addObjectElement(std::string && key , JsonVar && value)
{
    assert(m_type == JsonVar::OBJECT);
    if(m_val.object_p == nullptr)
        m_val.object_p = new JsonVar::Object;
    m_val.object_p->insert(std::move(std::make_pair(key, value)));
}

bool JsonVar::operator==(const JsonVar & jv)
{
    if(m_type != jv.m_type)
        return false;
    switch(m_type)
    {
        case JsonVar::JsonType::NULL_TYPE : return true;
        case JsonVar::JsonType::FALSE : return true;
        case JsonVar::JsonType::TRUE : return true;
        case JsonVar::JsonType::NUMBER : return m_val.num_val == jv.m_val.num_val;
        case JsonVar::JsonType::STRING : return strcmp(m_val.str.s, jv.m_val.str.s) == 0;
        case JsonVar::JsonType::ARRAY : 
            if(this->getArraySize() != jv.getArraySize())
                return false;
            if(m_val.array_p == nullptr)
                return true;
            for(size_t i = 0; i < this->getArraySize(); ++i)
            {
                if((*m_val.array_p)[i] != (*jv.m_val.array_p)[i])
                    return false;
            }
            return true;  
        case JsonVar::JsonType::OBJECT : 
            if(getObjectSize() != jv.getObjectSize())
                return false;
            if(m_val.object_p == nullptr)
                return true;
            for(auto it = m_val.object_p->begin(); it != m_val.object_p->end(); ++it)
            {
                auto it_temp = jv.m_val.object_p->find(it->first);
                if(it_temp != jv.m_val.object_p->end())
                {
                    if(it->second != it_temp->second)
                        return false;
                }   
                else
                    return false;
            }
            return true;
        default : // error type : should not happen
            throw std::exception();  
    }
}

bool JsonVar::operator!=(const JsonVar & jv)
{
    return !this->operator==(jv);
}

JsonVar & JsonVar::operator[](const std::string & key)
{
    assert(m_type == JsonVar::OBJECT);
    auto i = getObjectValue(key);
    assert(i != nullptr);
    return *i;
}

JsonVar & JsonVar::operator[](size_t index)
{
    assert(m_type == JsonVar::ARRAY && getArraySize() > index);
    return *getArrayElememt(index);
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
    if((jps = parseValue(&c, out_jv)) == Json::ParseStatus::OK)
    {
        parseWhitespace(&c);
        if(*c.json != '\0') 
        {
            // TODO : is it necessary?
            out_jv->setType(JsonVar::NULL_TYPE);
            return Json::ParseStatus::ROOT_NOT_SINGULAR;
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
    case '\0' : return Json::ParseStatus::EXPECT_VALUE;
    case 'f' : return parseFalse(c, out_jv);
    case 't' : return parseTrue(c, out_jv);
    case '\"' : return parseCStr(c, out_jv);
    case '[' : return parseArray(c, out_jv);
    case '{' : return parseObject(c, out_jv);
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
        return Json::ParseStatus::OK;
    }
    else
        return Json::ParseStatus::INVALID_VALUE;
}

Json::ParseStatus Json::parseFalse(Context *c, JsonVar * out_jv)
{
    assert( *c->json == 'f' );
    if(c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e')
    {
        out_jv->setType(JsonVar::JsonType::FALSE);
        c->json += 5;
        return Json::ParseStatus::OK;
    }
    else
        return Json::ParseStatus::INVALID_VALUE;
}

Json::ParseStatus Json::parseTrue(Context *c, JsonVar * out_jv)
{
    assert( *c->json == 't' );
    if(c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e')
    {
        out_jv->setType(JsonVar::JsonType::TRUE);
        c->json += 4;
        return Json::ParseStatus::OK;
    }
    else
        return Json::ParseStatus::INVALID_VALUE;
}

Json::ParseStatus Json::parseNumber(Context * c, JsonVar * out_jv)
{
    char *end = nullptr;
    // if we check number using index, 
    // it will make mistake or miss some error case. Like: -.1
    // if(c->json[0] == '+' || c->json[0] == '.' || isalpha(c->json[0]))
    //     return Json::ParseStatus::INVALID_VALUE;
    // if(c->json[0] == '0' && c->json[1] != '.' && c->json[1] != '\0')
    //     return Json::ParseStatus::ROOT_NOT_SINGULAR;

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
        if(!ISDIGIT1TO9(*p)) return Json::ParseStatus::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p); // jump pass the number
    }
    /* after zero should be '.' or nothing */
    if(*p == '.')
    {
        p++;
        if(!ISDIGIT(*p)) return Json::ParseStatus::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);
    }
    
    // check 'e' or 'E'
    if(*p == 'e' || *p == 'E')
    {
        p++;
        if(*p == '-' || *p == '+') p++; // 1e10 is correct
        if(!ISDIGIT(*p)) return Json::ParseStatus::INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);

    }

    // prase() with check the behind of *p
    // to see if there something left.
    // Like : 0123 (parse 0, left 123) 
    
    errno = 0;  // c-type error handling
    double temp = strtod(c->json, &end);    // TODO : strtod can be improved
    //ERANGE:Result too large
    //HUGE_VAL:positive double expression that indicates overflow
    if(errno == ERANGE && (temp == HUGE_VAL || temp == -HUGE_VAL)) 
        return Json::ParseStatus::NUMBER_TOO_BIG;

    c->json = p;
    out_jv->setType(JsonVar::NUMBER);
    out_jv->setNumberVal(temp);
    return Json::ParseStatus::OK;
}

Json::ParseStatus HandleAndReturnError(Context * c, JsonVar * out_jv, Json::ParseStatus status, size_t size)
{
    //c->elem_stack.clear();
    c->elem_stack.pop<char>(size);
    // TODO : is it necessary?
    out_jv->setType(JsonVar::NULL_TYPE);
    return status;
}

Json::ParseStatus Json::parseCStr(Context * c, JsonVar * out_jv)
{
    size_t size =  c->elem_stack.getSize(); // current size in stack;
    const char * p = c->json;
    assert(*p == '\"');
    u_int32_t u; // save utf-8 code
    for(;;)
    {
        p++;
        size_t temp_size = c->elem_stack.getSize() - size;
        switch(*p)
        {
            
            case '\"' : 
            {  
                out_jv->setType(JsonVar::STRING);
                out_jv->setCStr(reinterpret_cast<char *>(c->elem_stack.getData()) + size, temp_size);
                c->elem_stack.pop<char>(temp_size);
                c->json = p + 1;
                return Json::ParseStatus::OK;
            }
            case '\0' :
                return HandleAndReturnError(c, out_jv, Json::ParseStatus::MISS_QUOTATION_MARK, temp_size);
            case '\\' :
            {
                p++;
                switch(*p)
                {
                    case 'n' : *(c->elem_stack.push<char>()) = '\n'; break;
                    case '/' : *(c->elem_stack.push<char>()) = '/'; break;
                    case 'b' : *(c->elem_stack.push<char>()) ='\b'; break;
                    case 'f' : *(c->elem_stack.push<char>()) ='\f'; break;
                    case 'r' : *(c->elem_stack.push<char>()) ='\r'; break;
                    case 't' : *(c->elem_stack.push<char>()) ='\t'; break;
                    case '\\' : *(c->elem_stack.push<char>()) ='\\'; break;
                    case '\"' : *(c->elem_stack.push<char>()) ='\"'; break;
                    case 'u' :
                    {
                        p++;
                        if(!parseHex4(p, u))
                            return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_UNICODE_HEX, temp_size);
                        
                        // check UNICODE_SURROGATE
                        if(u >= 0xDC00)// low surrogate occur with a high surrogate
                            return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_UNICODE_SURROGATE, temp_size);
                        if(u >= 0xD800 && u <= 0xDBFF) // high surrogate
                        {
                            p += 4; // jump to next code. should begin with '\\'
                            if(*p != '\\' || *(p+1) != 'u')
                                return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_UNICODE_SURROGATE, temp_size);
                            u_int32_t low_s;
                            if(!parseHex4(p+2, low_s))
                                return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_UNICODE_SURROGATE, temp_size);
                            if(low_s < 0xDC00 || low_s > 0xDFFF)
                                return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_UNICODE_SURROGATE, temp_size);
                            
                            u = 0x10000 + ((u - 0xD800) << 10) + (low_s - 0xDC00);

                            p += 2; // jump to first X of \\uXXXX
                        }

                        assert(u >= 0 && u <= 0x10FFFF);
                        if (u >= 0 && u <= 0x007F)
                            *(c->elem_stack.push<char>()) = u;
                        else if (u >= 0x0080 && u <= 0x07FF)
                        {
                            // byte1 : 110x xxxx. 0xC0: 1100 0000.  0x1F:    1 1111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            *(c->elem_stack.push<char>()) = 0xC0 | ((u >> 6) & 0x1F);
                            *(c->elem_stack.push<char>()) =0x80 | (u & 0x3F);
                        }
                        else if (u >= 0x0800 && u <= 0xFFFF)
                        {
                            // byte1 : 1110 xxxx. 0xE0: 1110 0000.  0x0F:      1111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            // byte3 like byte2;
                            *(c->elem_stack.push<char>()) = 0xE0 | ((u >> 12) & 0x0F);
                            *(c->elem_stack.push<char>()) = 0x80 | ((u >> 6) & 0x3F);
                            *(c->elem_stack.push<char>()) = 0x80 | (u & 0x3F);
                        }
                        else
                        {
                            // byte1 : 1111 0xxx. 0xF0: 1111 0000.  0x08:      0111.
                            // byte2 : 10xx xxxx. 0x80: 1000 0000.  0x3F:   11 1111.
                            // byte3 byte4 like byte2;
                            *(c->elem_stack.push<char>()) = 0xF0 | ((u >> 18) & 0x08);
                            *(c->elem_stack.push<char>()) = 0x80 | ((u >> 12) & 0x3F);
                            *(c->elem_stack.push<char>()) = 0x80 | ((u >> 6) & 0x3F);
                            *(c->elem_stack.push<char>()) = 0x80 | (u & 0x3F);
                        }
                        p += 3; // p point at 'u', so jump to last number
                        break;
                    }
                    default : 
                        return HandleAndReturnError(c, out_jv, Json::ParseStatus::MISS_QUOTATION_MARK, temp_size);   
                }
                break;  
            }
            default : 
            {
                if(*p >= '\x00' && *p <= '\x1F')
                {
                    return HandleAndReturnError(c, out_jv, Json::ParseStatus::INVALID_STRING_CHAR, temp_size);
                }
                else
                    *(c->elem_stack.push<char>()) = *p;  
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
    //const char * p = c->json;
    //size_t size = 0; // store cur level array elememt nums
    assert(*c->json == '[');
    c->json++;
    parseWhitespace(c);
    JsonVar::Array * array = new JsonVar::Array;
    if(*c->json == ']')
    {
        c->json++;
        out_jv->setType(JsonVar::ARRAY);
        out_jv->setArray(array);
        return Json::ParseStatus::OK;
    }
    
    Json::ParseStatus ret;
    for(;;)
    {
        // bug : if c->elem_stack resize, pointer temp would bu invaild.
        //JsonVar * temp = c->elem_stack.emplace_back<JsonVar>();
        JsonVar temp;
        if((ret = parseValue(c, &temp)) != Json::ParseStatus::OK)
        {
            // !!!must destruct first, and then release the stack space occupied by current array's JsonVar
            delete array;
            return ret;
        }
        array->emplace_back(std::move(temp));
        parseWhitespace(c);
        if(*c->json == ',')
        {
            c->json++;
            parseWhitespace(c);   
        }
        else if(*c->json == ']')
        {
            out_jv->setType(JsonVar::ARRAY);
            out_jv->setArray(array);
            c->json++;
            return Json::ParseStatus::OK;
        }
        else
        {
            delete array;
            return Json::ParseStatus::MISS_COMMA_OR_SQUARE_BRACKET;
        }    
    }
    delete array; // ??
}

Json::ParseStatus Json::parseObject(Context * c, JsonVar * out_jv)
{
    assert(*c->json == '{');
    c->json++;
    //size_t count = 0; // num of pair<key, value>
    parseWhitespace(c);
    if(*c->json == '}')
    {
        c->json++;
        out_jv->setType(JsonVar::OBJECT); 
        // setType has set m_val to 0
        //out_jv->m_val.object_p = nullptr;
        return Json::ParseStatus::OK;
    }
    JsonVar::Object * temp = new JsonVar::Object;
    for(;;)
    {
        if(*c->json == '"')
        {
            JsonVar key;
            Json::ParseStatus ret;
            if((ret = parseValue(c, &key)) != Json::ParseStatus::OK)
            {
                delete temp;
                return ret;
            }

            parseWhitespace(c);
            if(*c->json == ':')
            {
                c->json++;
                parseWhitespace(c);
            }
            else
            {
                delete temp;
                return Json::ParseStatus::MISS_COLON;
            }

            JsonVar value;
            if((ret = parseValue(c, &value)) != Json::ParseStatus::OK)
            {
                delete temp;
                return ret;
            }
            temp->emplace(std::make_pair(std::string(key.getCStr()), std::move(value)));   
        }
        else
        {
            delete temp;
            return Json::ParseStatus::MISS_KEY;
        }
        
        
        parseWhitespace(c);
        if(*c->json == ',')
        {
            c->json++;
            parseWhitespace(c);
        }
        else if(*c->json == '}')
        {
            c->json++;
            out_jv->setType(JsonVar::OBJECT);
            out_jv->setObject(temp);
            temp = nullptr;
            return Json::ParseStatus::OK;
        }
        else
        {
            delete temp;
            return Json::ParseStatus::MISS_COMMA_OR_CURLY_BRACKET;
        }
        
    }
}

std::string Json::stringify(const JsonVar *in_jv)
{
    if(in_jv == nullptr)
        return std::string();
    Context c;
    stringifyValue(&c, in_jv);


    c.json = static_cast<char *>(c.elem_stack.getData());
    std::string ret(c.json, c.elem_stack.getSize());
    c.elem_stack.clear();
    return ret;
}

void Json::stringifyLiteral(char *buf, const char * str, size_t size)
{
    memcpy(buf, str, size);
}

void Json::stringifyValue(Context *c, const JsonVar *in_jv)
{
    switch(in_jv->getType())
    {
        case JsonVar::JsonType::NULL_TYPE : 
            stringifyLiteral(c->elem_stack.push<char>(4), "null", 4); break;
        case JsonVar::JsonType::FALSE : 
            stringifyLiteral(c->elem_stack.push<char>(5), "false", 5); break;
        case JsonVar::JsonType::TRUE : 
            stringifyLiteral(c->elem_stack.push<char>(4), "true", 4); break;
        case JsonVar::JsonType::NUMBER :  
        {
            char *buf = c->elem_stack.push<char>(STRINGIFY_NUM_BUF_SIZE);
            int length = sprintf(buf, "%.17g", in_jv->getNumberVal());
            c->elem_stack.pop<char>(STRINGIFY_NUM_BUF_SIZE - length);
            break;
        }
        case JsonVar::JsonType::STRING : 
        {
            stringifyString(c, in_jv->getCStr(), in_jv->getCStrLength());
            break;
        }
        case JsonVar::JsonType::ARRAY :
        {
            stringifyArray(c, in_jv);
            break;
        }
        case JsonVar::JsonType::OBJECT :
        {
            stringifyObject(c, in_jv);
            break;
        } 
        default :
        {
            // do nothing
            return;
        } 
    }
}

void Json::stringifyString(Context * c, const char * str, size_t length)
{
    size_t str_buf_size = length * 6 + 3;  // assume all of chars are \x00
    char * buf = c->elem_stack.push<char>(str_buf_size);
    char * p = buf;
    *p++ = '\"';
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
        'A', 'B', 'C', 'D', 'E', 'F' };
    for(size_t i = 0; i < length; ++i)
    {
        unsigned char ch = static_cast<unsigned char>(str[i]);
        switch(ch)
        {
            case '\n': stringifyLiteral(p, "\\n", 2); p += 2; break;
            //case '/': stringifyLiteral(p, "/", 1); p += 1; break;
            case '\b': stringifyLiteral(p, "\\b", 2); p += 2; break;
            case '\f': stringifyLiteral(p, "\\f", 2); p += 2; break;
            case '\r': stringifyLiteral(p, "\\r", 2); p += 2; break;
            case '\t': stringifyLiteral(p, "\\t", 2); p += 2; break;
            //case '\0': stringifyLiteral(p, "\\u0000", 6); p += 6; break;
            case '\"': stringifyLiteral(p, "\\\"", 2); p += 2; break;
            case '\\' : stringifyLiteral(p, "\\\\", 2); p += 2; break;
            default : 
            {
                if(ch < 0x20)
                {
                    stringifyLiteral(p, "\\u00", 4);
                    p += 4;
                    *p++ = hex_digits[ch >> 4];
                    *p++ = hex_digits[ch % 15];
                }
                else
                    *p++ = str[i];
                break;
            }
        }
    }
    *p++ = '\"';
    c->elem_stack.pop<char>(str_buf_size - (p - buf));
}

void Json::stringifyArray(Context * c, const JsonVar *in_jv)
{
    *c->elem_stack.push<char>() = '[';
    if(in_jv->getArraySize() == 0)
    {
        *c->elem_stack.push<char>() = ']';
        return;
    }
    for(size_t i = 0; i < in_jv->getArraySize(); ++i)
    {
        // TODO : check if there were Objects in array?
        stringifyValue(c, in_jv->getArrayElememt(i));
        *c->elem_stack.push<char>() = ',';
    }
    *c->elem_stack.top<char>() = ']';
}

void Json::stringifyObject(Context * c, const JsonVar *in_jv)
{
    *c->elem_stack.push<char>() = '{';
    if(in_jv->getObjectSize() == 0)
    {
        *c->elem_stack.push<char>() = '}';
        return;
    }  
    for(auto & kvpair : *in_jv->getObject())
    {
        stringifyString(c, kvpair.first.c_str(), kvpair.first.length());
        *c->elem_stack.push<char>() = ':';
        stringifyValue(c, &kvpair.second);
        *c->elem_stack.push<char>() = ',';
    }

    *c->elem_stack.top<char>() = '}';
}

bool Json::loadFile(const char * file_path, JsonVar & jv)
{
    std::ifstream ifile(file_path);//, std::ios::in|std::ios::binary
    if(!ifile.is_open())
        return false;
    std::stringstream in_text;
    in_text << ifile.rdbuf();
    ifile.close();
    if(Json::ParseStatus::OK != parse(in_text.str().c_str(), &jv))
        return false;
    else   
        return true;           
}

bool Json::saveFile(const char * save_path, JsonVar & jv)
{
    std::ofstream ofile(save_path);
    if(!ofile.is_open())
        return false;
    ofile << stringify(&jv);
    ofile.close();
    return true;
}
// -----------------------------------------------Json implement end !
} // namespace Toy