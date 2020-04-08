#include <cassert>
#include <cstdlib>  // strtod()
#include <cctype>   // for isalpha
#include "Json.h"


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
    m_val.bool_val = b;
}

// -----------------------------------------------JsonVar implement end !
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
            return Json::ROOT_NOT_SINGULAR;
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
    // TODO : validate number
    if(c->json[0] == '+' || c->json[0] == '.' || isalpha(c->json[0]))
        return Json::INVALID_VALUE;
    if(c->json[0] == '0' && c->json[1] != '.' && c->json[1] != '\0')
        return Json::ROOT_NOT_SINGULAR;
    double temp = strtod(c->json, &end);
    if(c->json == end || *(end - 1) == '.')
        return Json::INVALID_VALUE;
    c->json = end;
    out_jv->setType(JsonVar::NUMBER);
    out_jv->setNumberVal(temp);
    return Json::OK;
}

// -----------------------------------------------Json implement end !
} // namespace Toy