#ifndef _DICT_BUILDER_H
#define _DICT_BUILDER_H
#include <DAWG/BuildFactory.h>
#include <utils/Misc.h>
#include "RawDict.h"
#include <tag/UniSPTag.h>
#include <tag/UniMorphTag.h>
#include "MorphDict.h"
#include "SuffixDict.h"
#include "BuildDefs.h"
#include "DisambDict.h"
#include "PhemDict.h"


namespace build {

class DictBuilder {
private:
    static constexpr std::size_t MIN_PARA_COUNT = 2;
    static constexpr std::size_t MIN_FLEX_FREQ = 1;
    std::map<Paradigm, ParadigmOccurences> paras;
    StringToIndexBiMap prefs;
    TagToIndexBiMap tags;
    StringToIndexBiMap sufs;
    const std::size_t minFlexFreq;
    const std::size_t minParaCount;
    std::vector<EncodedParadigm> encPars;

    DictPtr loadClassicDict(const RawDict & rd, LoadFunc loader, FilterFunc filter) const;
    void mainDictLoader(std::map<std::string, ParaPairArray>& m, const WordsArray& w, const TagsArray& t) const;
    void suffixDictLoader(std::map<std::string, ParaPairArray>& m, const WordsArray& w, const TagsArray& t) const;
    void filterSuffixDict(std::map<std::string, ParaPairArray>& m) const;

public:
    DictBuilder(
        const std::map<Paradigm, ParadigmOccurences>& paras,
        const std::map<EncodedParadigm, std::size_t>& epars,
        const IntermediateParadigmsState & intermediateState,
        std::size_t minFlexFreq = MIN_FLEX_FREQ,
        std::size_t minParaCount = MIN_PARA_COUNT)
        : paras(paras)
        , prefs(intermediateState.prefixesMap)
        , tags(intermediateState.tagsMap)
        , sufs(intermediateState.suffixesMap)
        , minFlexFreq(minFlexFreq)
        , minParaCount(minParaCount)
        , encPars(epars.size()) {
        for (auto itr : epars) {
            encPars[itr.second] = itr.first;
        }
    }

    std::unique_ptr<MorphDict> buildMorphDict(const RawDict & rd);
    std::unique_ptr<SuffixDict> buildSuffixDict(const RawDict & rd);
};

std::unique_ptr<DisambDict> buildDisambDict(std::istream & is);
void buildPhemDict(std::unique_ptr<PhemDict>& dict, std::istream& is, std::shared_ptr<RawDict> rd);

using PrefixDict = std::set<utils::UniString>;
PrefixDict loadPrefixDict(std::istream & is);
}
#endif
