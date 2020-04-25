#include "DictBuilder.h"
namespace build {

std::unique_ptr<MorphDict> DictBuilder::buildMorphDict(const RawDict & rd) {
    LoadFunc dictLoader = std::bind(&DictBuilder::mainDictLoader, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    auto mainDict = loadClassicDict(rd, dictLoader, [](std::map<std::string, ParaPairArray>&) {});
    return utils::make_unique<MorphDict>(encPars, mainDict, prefs, tags, sufs);
}

void DictBuilder::mainDictLoader(std::map<std::string, ParaPairArray>& m, const WordsArray& w, const TagsArray& t) const {
    Paradigm p = parseOnePara(w, t);
    for (std::size_t i = 0; i < w.size(); ++i) {
        m[w[i].getRawString()].data.emplace_back(ParaPair{paras.at(p).paradigmNumber, i, paras.at(p).paradigmFrequency});
    }
}

DictPtr DictBuilder::loadClassicDict(
    const RawDict & rd, LoadFunc loader, FilterFunc filter) const {

    dawg::BuildFactory<ParaPairArray> factory;
    std::map<std::string, ParaPairArray> allData;
    std::size_t counter = 0;
    for (std::size_t i = 0; i < rd.size(); ++i) {
        auto [words, tags] = rd[i];
        if (!words.empty()) {
            loader(allData, words, tags);
        }
        counter += 1;
        if (counter % 1000 == 0) {
            std::cerr << "Dict loading: " << counter << std::endl;
        }
    }
    filter(allData);
    for (auto itr : allData) {
        factory.insertOrLink(itr.first, itr.second);
    }
    return factory.build();
}

void DictBuilder::suffixDictLoader(std::map<std::string, ParaPairArray>& m, const WordsArray& w, const TagsArray& t) const {
    Paradigm p = parseOnePara(w, t);
    base::UniSPTag sp = p[0].sp;
    if (paras.at(p).paradigmFrequency < minParaCount || base::UniSPTag::getStaticSPs().count(sp)) {
        return;
    }
    std::size_t paranumCur = paras.at(p).paradigmNumber;
    for (std::size_t i = 0; i < w.size(); ++i) {
        for (std::size_t j = 1; j <= 5; ++j) {
            if (j < w[i].length()) {
                utils::UniString cut = w[i].rcut(j);
                std::string raw = cut.getRawString();

                bool found = false;
                for (auto& para : m[raw].data) {
                    if (para.paraNum == paranumCur && para.formNum == i) {
                        para.freq++;
                        found = true;
                    }
                }
                if (!found) {
                    m[raw].data.push_back(ParaPair{paranumCur, i, 1});
                }
            }
        }
    }
}

void DictBuilder::filterSuffixDict(std::map<std::string, ParaPairArray>& m) const {
    for (auto& pair : m) { // clear not frequent
        for (auto itr = pair.second.data.begin(); itr != pair.second.data.end();) {
            if (itr->freq <= minFlexFreq) {
                itr = pair.second.data.erase(itr);
            } else {
                ++itr;
            }
        }
    }
    /// TODO some bug hidden here
    //for (auto& pair : m) {
    //    std::map<base::UniSPTag, std::pair<std::size_t, std::size_t>> counter;
    //    for (std::size_t i = 0; i < pair.second.data.size(); ++i) {
    //        ParaPair p = pair.second.data[i];
    //        EncodedLexemeGroup g = encPars[p.paraNum][p.formNum];
    //        MorphTagPair tp = tags.right.at(g.tagId);
    //        if (counter.count(tp.sp) == 0 || counter[tp.sp].first < p.freq) {
    //            counter[tp.sp] = std::make_pair(p.freq, i);
    //        }
    //    }
    //    ParaPairArray newParas;
    //    for (auto itr : counter) {
    //        for (std::size_t i = 0; i < pair.second.data.size(); ++i) {
    //            if (pair.second.data[i].paraNum == pair.second.data[itr.second.second].paraNum) {
    //                newParas.data.push_back(pair.second.data[i]);
    //            }
    //        }
    //    }
    //    pair.second.data = newParas.data;
    //}
    for (auto itr = m.begin(); itr != m.end();) {
        if (itr->second.data.empty()) {
            itr = m.erase(itr);
        } else {
            itr++;
        }
    }
}

std::unique_ptr<SuffixDict> DictBuilder::buildSuffixDict(const RawDict& rd) {
    LoadFunc dictLoader = std::bind(&DictBuilder::suffixDictLoader, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FilterFunc filter = std::bind(&DictBuilder::filterSuffixDict, this, std::placeholders::_1);
    auto suffixDict = loadClassicDict(rd, dictLoader, filter);
    return utils::make_unique<SuffixDict>(encPars, suffixDict);
}

PrefixDict loadPrefixDict(std::istream & is)
{
    std::string row;
    std::set<utils::UniString> result;
    while (std::getline(is, row)) {
        result.insert(utils::UniString(row));
    }
    return result;
}

std::unique_ptr<DisambDict> buildDisambDict(std::istream & is) {
    std::string row;
    std::map<std::string, std::size_t> counter;

    std::size_t ccc = 0;
    while (std::getline(is, row)) {
        boost::trim(row);
        ccc++;
        std::cerr << ccc << std::endl;
        if (row.empty() || row == "#")
            continue;
        std::vector<std::string> parts;
        boost::split(parts, row, boost::is_any_of("\t"));
        utils::UniString word(parts[1]);
        base::UniSPTag sp = base::UniSPTag::X;
        base::UniMorphTag mt = base::UniMorphTag::UNKN;
        std::tie(sp, mt) = getTags<base::UniSPTag, base::UniMorphTag>(parts[3] + "|" + parts[4]);
        if (sp == base::UniSPTag::X) /// something undefined
            continue;
        std::string rawSp = to_raw_string(sp);
        std::string rawMt = to_raw_string(mt);

        counter[word.toUpperCase().replace(u'ё', u'е').getRawString() + DISAMBIG_SEPARATOR + rawSp + DISAMBIG_SEPARATOR + rawMt] += 1;
    }

    dawg::BuildFactory<std::size_t> factory;
    for (auto itr : counter) {
        factory.insertOrLink(itr.first, itr.second);
    }
    DisambDictPtr dct = factory.build();
    return utils::make_unique<DisambDict>(dct);
}

std::map<utils::UniString, std::size_t> turnSortedSequenceIntoCountedMap(std::set<utils::UniString>&& data) {
    std::map<utils::UniString, std::size_t> result;
    for (auto itr = data.begin(); itr != data.end(); ++itr) {
        const utils::UniString& word = *itr;
        for (std::size_t i = 0; i < word.length(); ++i) {
            utils::UniString cut = word.subString(0, word.length() - i);
            if (result.count(cut)) { // if we already encounded some cut, then we already encounded all subcuts
                break;
            }
            result[cut] = 1;
            char16_t prev;
            for (auto innerItr = std::next(itr); innerItr != data.end(); ++innerItr) {
                if (innerItr->length() <= cut.length())
                    break; // it cannot' starts with cut
                char16_t afterCut = (*innerItr)[cut.length()];
                if (innerItr->startsWith(cut)) {
                    if (afterCut != prev) { // inc only if next symbol is different
                        result[cut]++;
                        prev = afterCut;
                    }
                } else {
                    break; //all sequential strings are not starting with cut
                }
            }
        }
    }
    return result;
}
InnerCounterPhemDictPtr mapToFactory(std::map<utils::UniString, std::size_t>&& mp) {
    dawg::BuildFactory<std::size_t> factory;
    for (auto itr : mp) {
        factory.insertOrLink(itr.first.getRawString(), itr.second);
    }
    return factory.build();
}
std::tuple<InnerCounterPhemDictPtr, InnerCounterPhemDictPtr> buildCountPhemDict(std::shared_ptr<RawDict> rd) {
    std::set<utils::UniString> forwardSet, backwardSet;
    for (std::size_t i = 0; i < rd->size(); ++i) {
        const WordsArray & words = rd->operator[](i).words;
        for (const auto& wrd : words) {
            forwardSet.insert(wrd);
            backwardSet.insert(wrd.reverse());
        }
    }
    std::map<utils::UniString, std::size_t> fmap = turnSortedSequenceIntoCountedMap(std::move(forwardSet));
    InnerCounterPhemDictPtr forward = mapToFactory(std::move(fmap));
    std::map<utils::UniString, std::size_t> bmap = turnSortedSequenceIntoCountedMap(std::move(backwardSet));
    InnerCounterPhemDictPtr backward = mapToFactory(std::move(bmap));
    return std::make_tuple(forward, backward);
}
}
