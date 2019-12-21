#include "UniSPTag.h"
namespace base {
static const boost::bimap<uint128_t, std::string> UNI_SP_MAP =
boost::assign::list_of<boost::bimap<uint128_t, std::string>::relation>
(0x00     ,     "X")
(0x01     ,     "ADJ")
(0x02     ,     "ADV")
(0x04     ,     "INTJ")
(0x08     ,     "NOUN")
(0x10     ,     "PROPN")
(0x20     ,     "VERB")
(0x40     ,     "ADP")
(0x80     ,     "AUX")
(0x100    ,     "CONJ")
(0x200    ,     "SCONJ")
(0x400    ,     "DET")
(0x800    ,     "NUM")
(0x1000   ,     "PART")
(0x2000   ,     "PRON")
(0x4000   ,     "PUNCT")
(0x8000   ,     "H")
(0x10000  ,     "R")
(0x20000  ,     "Q");

const UniSPTag UniSPTag::X(uint128_t(0x00   ));
const UniSPTag UniSPTag::ADJ(uint128_t(0x01   ));
const UniSPTag UniSPTag::ADV(uint128_t(0x02   ));
const UniSPTag UniSPTag::INTJ(uint128_t(0x04   ));
const UniSPTag UniSPTag::NOUN(uint128_t(0x08   ));
const UniSPTag UniSPTag::PROPN(uint128_t(0x10   ));
const UniSPTag UniSPTag::VERB(uint128_t(0x20   ));
const UniSPTag UniSPTag::ADP(uint128_t(0x40   ));
const UniSPTag UniSPTag::AUX(uint128_t(0x80   ));
const UniSPTag UniSPTag::CONJ(uint128_t(0x100  ));
const UniSPTag UniSPTag::SCONJ(uint128_t(0x200  ));
const UniSPTag UniSPTag::DET(uint128_t(0x400  ));
const UniSPTag UniSPTag::NUM(uint128_t(0x800  ));
const UniSPTag UniSPTag::PART(uint128_t(0x1000 ));
const UniSPTag UniSPTag::PRON(uint128_t(0x2000 ));
const UniSPTag UniSPTag::PUNCT(uint128_t(0x4000 ));
const UniSPTag UniSPTag::H(uint128_t(0x8000 ));
const UniSPTag UniSPTag::R(uint128_t(0x10000 ));
const UniSPTag UniSPTag::Q(uint128_t(0x10000 ));

const std::vector<UniSPTag> UniSPTag::inner_runner = {
	X,
	ADJ,
	ADV,
	INTJ,
	NOUN,
	PROPN,
	VERB,
	ADP,
	AUX,
	CONJ,
	SCONJ,
	DET,
	NUM,
	PART,
	PRON,
    PUNCT,
    H,
    R,
    Q,
};

UniSPTag::UniSPTag(uint128_t val): ITag(val, &UNI_SP_MAP) {}

UniSPTag::UniSPTag(const std::string &val): ITag(val, &UNI_SP_MAP) {}

UniSPTag::UniSPTag(): ITag((uint128_t)0, &UNI_SP_MAP) {}

}