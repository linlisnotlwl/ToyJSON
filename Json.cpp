#include <cassert>

#include "Json.h"


namespace Toy
{
// -----------------------------------------------xx implement start :
// -----------------------------------------------xx implement end !


// -----------------------------------------------Json implement start :

Json::ParseStatus Json::parse(const char *json_text, JsonVar * out_jv)
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
    default : return Json::INVALID_VALUE;
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

// -----------------------------------------------Json implement end !
} // namespace Toy