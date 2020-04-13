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

Json::ParseStatus Json::parseCStr(Context * c, JsonVar * out_jv)
{
    const char * p = c->json;
    assert(*p == '\"');
    //int count = 1;
    for(;;)
    {
        p++;
        switch(*p)
        {
            case '\"' : 
            {
                
                out_jv->setType(JsonVar::STRING);
                out_jv->setCStr(c->char_stack.data(), c->char_stack.size());
                c->char_stack.clear();
                c->json = p + 1;
                return Json::ParseStatus::OK;

            }
            case '\0' :
            {
                c->char_stack.clear();
                out_jv->setType(JsonVar::NULL_TYPE);
                return Json::ParseStatus::MISS_QUOTATION_MARK;
            }
            case '\\' :
            {
                p++;
                if (*p == '/' || *p == 'b' || *p == 'f' || *p == 'n' 
                || *p == 'r' || *p == 't' || *p == '\\' || *p == '\"')
                {
                    c->char_stack.push_back(transEscape(*p));   
                }
                else
                {
                    c->char_stack.clear();
                    out_jv->setType(JsonVar::NULL_TYPE);
                    return Json::ParseStatus::INVALID_STRING_ESCAPE;
                }
                break;  
            }
            default : 
            {
                if(*p >= '\x00' && *p <= '\x1F')
                {
                    c->char_stack.clear();
                    out_jv->setType(JsonVar::NULL_TYPE);
                    return Json::ParseStatus::INVALID_STRING_CHAR;
                }
                else
                    c->char_stack.push_back(*p);  
            }
        }
    }
}
const char Json::transEscape(const char & c)
{
    if(c == 'n')
        return '\n';
    if(c == '/')
        return '/';
    if(c == 'b')
        return '\b';
    if(c == 'f')
        return '\f';
    if(c == 'r')
        return '\r';
    if(c == 't')
        return '\t';
    if(c == '\\')
        return '\\';
    if(c == '\"')
        return '\"';
    return ' '; // FIXME : something to return 
}
// -----------------------------------------------Json implement end !
} // namespace Toy