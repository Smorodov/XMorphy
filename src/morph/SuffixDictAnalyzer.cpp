#include "SuffixDictAnalyzer.h"
namespace analyze {
std::vector<ParsedPtr> SuffixDictAnalyzer::analyze(const utils::UniString& str) const {
    if (PrefixAnalyzer::isDictWord(str)) {
        return PrefixAnalyzer::analyze(str);
    }
    build::ParaPairArray rawInfo = sufDict->getCandidates(str);
    std::vector<std::tuple<build::LexemeGroup, build::AffixPair, std::size_t>> result;
    dict->getClearForms(rawInfo, result);
    std::vector<ParsedPtr> r = DictMorphAnalyzer::analyze(str, result);
    for (auto ptr : r) {
        ptr->at = base::AnalyzerTag::SUFF;
    }
    return r;
}

std::vector<ParsedPtr> SuffixDictAnalyzer::synthesize(const utils::UniString& str, const base::MorphTag& t) const {
    if (PrefixAnalyzer::isDictWord(str)) {
        return PrefixAnalyzer::synthesize(str, t);
    }
    build::ParaPairArray rawInfo = sufDict->getCandidates(str);
    std::map<build::Paradigm, std::size_t> result;
    dict->getParadigmsForForm(rawInfo, result);
    std::vector<ParsedPtr> r = DictMorphAnalyzer::synthesize(str, t, result);
    for (auto ptr : r) {
        ptr->at = base::AnalyzerTag::SUFF;
    }
    return r;
}

std::vector<ParsedPtr> SuffixDictAnalyzer::synthesize(const utils::UniString& str, const base::MorphTag& given, const base::MorphTag& req) const {
    if (PrefixAnalyzer::isDictWord(str)) {
        return PrefixAnalyzer::synthesize(str, given, req);
    }
    build::ParaPairArray rawInfo = sufDict->getCandidates(str);
    std::map<build::Paradigm, std::size_t> result;
    dict->getParadigmsForForm(rawInfo, result);
    std::vector<ParsedPtr> r = DictMorphAnalyzer::synthesize(str, given, req, result);
    for (auto ptr : r) {
        ptr->at = base::AnalyzerTag::SUFF;
    }
    return r;
}
}