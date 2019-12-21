#include "TokenTypeTag.h"
namespace base {
static const boost::bimap<uint128_t, std::string> NAME_MAP =
    boost::assign::list_of<boost::bimap<uint128_t, std::string>::relation>
    (0x00     ,     "UNKN")
    (0x01     ,     "WORD")
    (0x02     ,     "PNCT")
    (0x04     ,     "SEPR")
    (0x08     ,     "NUMB")
    (0x10     ,     "WRNM")
    (0x20     ,     "HIER");

const TokenTypeTag TokenTypeTag::UNKN(uint128_t(0x00));
const TokenTypeTag TokenTypeTag::WORD(uint128_t(0x01));
const TokenTypeTag TokenTypeTag::PNCT(uint128_t(0x02));
const TokenTypeTag TokenTypeTag::SEPR(uint128_t(0x04));
const TokenTypeTag TokenTypeTag::NUMB(uint128_t(0x08));
const TokenTypeTag TokenTypeTag::WRNM(uint128_t(0x10));
const TokenTypeTag TokenTypeTag::HIER(uint128_t(0x20));

const std::vector<TokenTypeTag> TokenTypeTag::inner_runner = {
    UNKN,
    WORD,
    PNCT,
    SEPR,
    NUMB,
    WRNM,
    HIER,
};

TokenTypeTag::TokenTypeTag(uint128_t val):ITag(val, &NAME_MAP) {}
TokenTypeTag::TokenTypeTag(const std::string &val):ITag(val, &NAME_MAP) {}

}