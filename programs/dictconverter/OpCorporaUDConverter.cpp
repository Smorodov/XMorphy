#include "OpCorporaUDConverter.h"

#define MT(X) base::MorphTag::X
#define UMT(X) base::UniMorphTag::X
#define SP(X) base::SpeechPartTag::X
#define USP(X) base::UniSPTag::X
using namespace base;

std::ostream& operator<<(std::ostream& os, const ConvertMorphInfo& info) {
    os << "[NF: " << info.normalForm << "] "
       << "SP:" << info.sp << " MT:" << info.tag
       << " USP:" << info.usp << " UMT:" << info.utag << std::endl;
    return os;
}

void OpCorporaUDConverter::adjRule(ConvertMorphInfo& mi, const SpeechPartTag& sp, MorphTag& mt) const {
    if (sp == SP(ADJF) && dets.count(mi.normalForm)) {
        mi.usp = USP(DET);
    } else if (sp == SP(ADJF) || sp == SP(PRTF)) {
        mi.usp = USP(ADJ);
        mt.resetIfContains(MT(past));
        mt.resetIfContains(MT(pres));
        mt.resetIfContains(MT(futr));
        mt.resetIfContains(MT(actv));
        mt.resetIfContains(MT(pssv));
        mi.utag |= UMT(Pos);
    } else if (sp == SP(ADJS) || sp == SP(PRTS)) {
        mi.usp = USP(ADJ);
        mt.resetIfContains(MT(past));
        mt.resetIfContains(MT(pres));
        mt.resetIfContains(MT(futr));
        mt.resetIfContains(MT(actv));
        mt.resetIfContains(MT(pssv));
        mi.utag |= UMT(Brev);
        mi.utag |= UMT(Pos);
    } else if (sp == SP(NUMR) && mt.resetIfContains(MT(Anum))) {
        mi.usp = USP(ADJ);
        mi.tag |= UMT(Pos);
    }
}

void OpCorporaUDConverter::adjRule(ConvertWordForm & wf) const {
    bool predFound = false, adjsFound = false;
    std::set<ConvertMorphInfo>& infos = wf.infos;
    utils::UniString wfUpper = wf.wordForm.toUpperCase();
    for (auto& mi : infos) {
        bool wfCount = prons.count(wfUpper);
        bool nfCount = prons.count(mi.normalForm);
        if (mi.sp == SP(PRED))
            predFound = true;
        if (mi.sp == SP(ADJS))
            adjsFound = true;
        if (mi.sp == SP(ADJF) && (wfCount || nfCount)) {
            utils::UniString form = wfCount ? wfUpper : mi.normalForm;
            ConvertMorphInfo newMi{
                form,
                MT(UNKN),
                SP(UNKN),
                UMT(UNKN),
                USP(PRON),
            };
            base::MorphTag t = mi.tag;
            restRuleMT(newMi, t);
            infos.insert(newMi);
        }
    }
    if (adjsFound && predFound) {
        for (auto itr = infos.begin(); itr != infos.end();) {
            if (itr->sp == SP(PRED)) {
                itr = infos.erase(itr);
            } else {
                itr++;
            }
        }
    }
    if (predFound && !adjsFound && fakeAdjs.count(wfUpper)) {
        ConvertMorphInfo fakeInfo{
            wfUpper,
            MT(UNKN),
            SP(UNKN),
            UMT(Brev),
            USP(ADJ),
        };
        fakeInfo.utag |= UMT(Pos) | UMT(Neut) | UMT(Sing);
        infos.insert(fakeInfo);
    }
}

void OpCorporaUDConverter::verbRule(ConvertMorphInfo & mi, const SpeechPartTag & sp, MorphTag & mt, bool tsya) const {
    if (sp == SP(VERB)) {
        mi.usp = USP(VERB);
        mi.utag |= UMT(Fin);
        if (tsya) {
            mi.utag |= UMT(Mid);
        } else if (!(mt & MT(pssv))) {
            mi.utag |= UMT(Act);
        }
    } else if (sp == SP(INFN)) {
        mi.usp = USP(VERB);
        mi.utag |= UMT(Inf);
    } else if (sp == SP(GRND)) {
        mi.usp = USP(VERB);
        mi.utag |= UMT(Conv);
        if (tsya) {
            mi.utag |= UMT(Mid);
        } else if (!(mt & MT(pssv))) {
            mi.utag |= UMT(Act);
        }
    } else if (sp == SP(PRED) && (mi.normalForm.toUpperCase() == utils::UniString("НЕТ") || mi.normalForm.toUpperCase() == utils::UniString("НЕТУ"))) {
        mt.resetIfContains(MorphTag::pres);
        mi.usp = USP(VERB);
        mi.utag |= UMT(Ind) | UMT(Sing) | UMT(_3) | UMT(Pres) | UMT(Fin);
    }
}

void OpCorporaUDConverter::compRule(ConvertWordForm & wf) const {
    static utils::UniString uzhe = utils::UniString("УЖЕ");
    bool compFound = false;
    bool isYje = wf.wordForm.toUpperCase() == uzhe;
    auto & info = wf.infos;
    for (auto itr = info.begin(); itr != info.end();) {
        if (itr->sp == SP(COMP) && !isYje) {
            utils::UniString nf = itr->normalForm;
            info.erase(itr);
            info.insert(ConvertMorphInfo{
                nf,
                MT(UNKN),
                SP(UNKN),
                UMT(Cmp),
                USP(ADJ),
            });
            info.insert(ConvertMorphInfo{
                wf.wordForm,
                MT(UNKN),
                SP(UNKN),
                UMT(Cmp),
                USP(ADV),
            });
            compFound = true;
            break;
        } else if (itr->sp == SP(COMP) && isYje) {
            info.erase(itr);
            info.insert(ConvertMorphInfo{
                wf.wordForm,
                MT(UNKN),
                SP(UNKN),
                UMT(UNKN),
                USP(ADV),
            });
            info.insert(ConvertMorphInfo{
                wf.wordForm,
                MT(UNKN),
                SP(UNKN),
                UMT(UNKN),
                USP(PART),
            });
            break;
        } else {
            ++itr;
        }
    }

    if (compFound) {
        for (auto itr = info.begin(); itr != info.end();) {
            if (itr->sp == SP(ADVB) && info.size() > 1) {
                itr = info.erase(itr);
            } else {
                ++itr;
            }
        }
    }
}

void OpCorporaUDConverter::restRuleSP(
    ConvertMorphInfo& mi,
    const SpeechPartTag& sp,
    MorphTag& mt,
    const utils::UniString& wf) const
{
    if (sp == SP(NOUN)) {
        mi.usp = USP(NOUN);
    } else if (sp == SP(ADVB)) {
        mi.usp = USP(ADV);
        mi.utag |= UMT(Pos);
    } else if (sp == SP(PRED)) {
        mt.resetIfContains(MT(pres));
        mt.resetIfContains(MT(futr));
        mt.resetIfContains(MT(past));
        mi.usp = USP(ADV);
        mi.utag |= UMT(Pos);
    } else if (sp == SP(NUMR)) {
        mi.usp = USP(NUM);
    } else if (sp == SP(INTJ)) {
        mi.usp = USP(INTJ);
    } else if (sp == SP(PREP) && adps.count(mi.normalForm)) {
        mi.usp = USP(ADP);
        if (mi.tag.resetIfContains(MT(Vpre))) {
            mi.normalForm = wf.toUpperCase();
        }
    } else if (sp == SP(PRCL) && parts.count(mi.normalForm)) {
        mi.usp = USP(PART);
    } else if (sp == SP(CONJ) && conjs.count(mi.normalForm)) {
        mi.usp = USP(CONJ);
        mt = MT(UNKN);
    } else if (sp == SP(NPRO) && prons.count(mi.normalForm)) {
        mi.usp = USP(PRON);
    }

}
namespace {
void replaceOrInsert(base::UniSPTag sp, const utils::UniString& wf, std::vector<ConvertMorphInfo>& infos, std::vector<std::size_t>& Xs) {
    if (!Xs.empty()) {
        infos[Xs.back()].usp = sp;
        if (sp == USP(CONJ)
            || sp == USP(ADP)
            || sp == USP(PART)
            || sp == USP(H)
            || sp == USP(INTJ)) {
            infos[Xs.back()].utag = UMT(UNKN);
        }
        Xs.pop_back();
    } else {
        infos.push_back(ConvertMorphInfo{wf, MT(UNKN), SP(UNKN), UMT(UNKN), sp});
    }
}
}

void OpCorporaUDConverter::staticRule(
    const utils::UniString& wordform,
    const utils::UniString& upperwf,
    std::vector<ConvertMorphInfo>& infos) const {

    bool hasAdp = false, hasPart = false, hasConj = false, hasPron = false, hasDet = false, hasH = false;
    std::vector<std::size_t> Xs;
    for (std::size_t i = 0; i < infos.size(); ++i) {
        if (infos[i].usp == USP(X)) {
            Xs.push_back(i);
            continue;
        }
        hasAdp = hasAdp ? true : infos[i].usp == USP(ADP);
        hasPart = hasPart ? true : infos[i].usp == USP(PART);
        hasConj = hasConj ? true : infos[i].usp == USP(CONJ);
        hasPron = hasPron ? true : infos[i].usp == USP(PRON);
        hasDet = hasDet ? true : infos[i].usp == USP(DET);
        hasH = hasH ? true : infos[i].usp == USP(H);
    }
    if ((adps.count(wordform) || adps.count(upperwf)) && !hasAdp) {
        replaceOrInsert(USP(ADP), upperwf, infos, Xs);
    }
    if ((parts.count(wordform) || parts.count(upperwf)) && !hasPart) {
        replaceOrInsert(USP(PART), upperwf, infos, Xs);
    }
    if ((conjs.count(wordform) || conjs.count(upperwf)) && !hasConj) {
        replaceOrInsert(USP(CONJ), upperwf, infos, Xs);
    }
    if ((prons.count(wordform) || prons.count(upperwf)) && !hasPron) {
        replaceOrInsert(USP(PRON), upperwf, infos, Xs);
    }
    if ((dets.count(wordform) || dets.count(upperwf)) && !hasDet) {
        replaceOrInsert(USP(DET), upperwf, infos, Xs);
    }
    if ((Hs.count(wordform) || Hs.count(upperwf)) && !hasH) {
        replaceOrInsert(USP(H), upperwf, infos, Xs);
    }
    if (wordform.contains(utils::UniCharacter("_"))) {
        for (auto itr = infos.begin(); itr != infos.end();) {
            if (itr->usp != USP(DET) && itr->usp != USP(PART) && itr->usp != USP(CONJ) && itr->usp != USP(PRON) && itr->usp != USP(ADP) && itr->usp != USP(H)) {
                itr = infos.erase(itr);
            } else {
                ++itr;
            }
        }
        if (infos.empty()) {
            infos.push_back(ConvertMorphInfo{
                upperwf,
                MT(UNKN),
                SP(UNKN),
                UMT(Pos),
                USP(ADV),
            });
        }
    }
}

void OpCorporaUDConverter::restRuleMT(ConvertMorphInfo& mi, MorphTag& mt) const {
    static const std::map<MorphTag, UniMorphTag> SIMPLE_MAPING = {
        {MT(masc), UMT(Masc)},
        {MT(femn), UMT(Fem)},
        {MT(neut), UMT(Neut)},
        //    {MT(Ms_f), UMT(Masc)},
        {MT(plur), UMT(Plur)},
        {MT(sing), UMT(Sing)},
        {MT(Pltm), UMT(Plur)},
        {MT(Sgtm), UMT(Sing)},
        {MT(nomn), UMT(Nom)},
        {MT(gent), UMT(Gen)},
        {MT(datv), UMT(Dat)},
        {MT(accs), UMT(Acc)},
        {MT(ablt), UMT(Ins)},
        {MT(loct), UMT(Loc)},
        {MT(voct), UMT(Voc)},
        {MT(gen1), UMT(Gen)},
        {MT(gen2), UMT(Gen)},
        {MT(acc2), UMT(Nom)},
        {MT(loc1), UMT(Loc)},
        {MT(loc2), UMT(Loc)},
        {MT(anim), UMT(Anim)},
        {MT(inan), UMT(Inan)},
        {MT(Supr), UMT(Pos)},
        {MT(per1), UMT(_1)},
        {MT(per2), UMT(_2)},
        {MT(per3), UMT(_3)},
        {MT(pres), UMT(Notpast)},
        {MT(past), UMT(Past)},
        {MT(futr), UMT(Notpast)},
        {MT(indc), UMT(Ind)},
        {MT(impr), UMT(Imp)},
        {MT(actv), UMT(Act)},
    };

    for (auto itr = MorphTag::begin(); itr != MorphTag::end(); ++itr) {
        if (*itr & mt) {
            if (SIMPLE_MAPING.count(*itr)) {
                mi.utag |= SIMPLE_MAPING.at(*itr);
            }
        }
    }
}

void OpCorporaUDConverter::convert(ConvertWordForm & wf) const {
    std::set<ConvertMorphInfo> & sInfos = wf.infos;
    if (wf.tokenTag == base::TokenTypeTag::PNCT) {
        sInfos.clear();
        sInfos.insert(ConvertMorphInfo{
            wf.wordForm,
            MT(UNKN),
            SP(UNKN),
            UMT(UNKN),
            USP(PUNCT),
        });
        return;
    } else if (wf.tokenTag == base::TokenTypeTag::NUMB) {
        sInfos.clear();
        base::UniMorphTag res = UMT(UNKN);
        if (wf.graphemTag & (base::GraphemTag::BINARY | base::GraphemTag::DECIMAL | base::GraphemTag::OCT)) {
            res |= UMT(Digit);
        }
        sInfos.insert(ConvertMorphInfo{
            wf.wordForm,
            MT(UNKN),
            SP(NUMR),
            res,
            USP(NUM),
        });
        return;
    }
    adjRule(wf);
    compRule(wf);

    const utils::UniString upWf = wf.wordForm.toUpperCase();
    bool tsya = upWf.endsWith(utils::UniString("СЯ")) || upWf.endsWith(utils::UniString("СЬ"));
    std::vector<ConvertMorphInfo> infos(sInfos.begin(), sInfos.end());
    std::set<utils::UniString> nfs;
    for (ConvertMorphInfo & mi : infos) {
        if (mi.usp != USP(X))
            continue;
        base::SpeechPartTag sp = mi.sp;
        base::MorphTag mt = mi.tag;
        mi.usp = base::UniSPTag::X;
        mi.utag = base::UniMorphTag::UNKN;
        adjRule(mi, sp, mt);
        verbRule(mi, sp, mt, tsya);
        restRuleSP(mi, sp, mt, upWf);
        restRuleMT(mi, mt);
        nfs.insert(mi.normalForm);
        if (mi.normalForm == utils::UniString("?")) {
            mi.normalForm = upWf;
        }
    }
    for (auto str : nfs) {
        staticRule(str, upWf, infos);
    }

    if (infos.empty())
        throw std::runtime_error("Removed all infos for '" + wf.wordForm.getRawString() + "'");

    if (std::all_of(infos.begin(), infos.end(), [](const ConvertMorphInfo & mi) {
                return mi.usp == USP(X);}))
    {
        for (auto & info : infos)
        {
            if (info.usp == base::UniSPTag::X) {
                if (info.sp == SP(NOUN))
                    info.usp = USP(NOUN);
                else if (info.sp == SP(PRCL))
                    info.usp = USP(PART);
                else if (info.sp == SP(CONJ))
                    info.usp = USP(CONJ);
                else if (info.sp == SP(NPRO))
                    info.usp = USP(PRON);
                else if (info.sp == SP(INTJ))
                    info.usp = USP(INTJ);
                else if (info.sp == SP(PREP))
                    info.usp = USP(ADP);
                else
                    throw std::runtime_error("Cannot convert '" + upWf.getRawString() + "' with speech part '" + to_string(info.sp) + "'");
            }
        }
    }

    wf.infos = std::set<ConvertMorphInfo>(infos.begin(), infos.end());
}

OpCorporaUDConverter::OpCorporaUDConverter(const std::string& confpath) {
    pt::ptree root;
    pt::read_json(confpath, root);
    parseTag("ADP", adps, root);
    parseTag("CONJ", conjs, root);
    parseTag("DET", dets, root);
    parseTag("H", Hs, root);
    parseTag("PART", parts, root);
    parseTag("PRON", prons, root);
    parseTag("FAKE_ADJ", fakeAdjs, root);
}

void OpCorporaUDConverter::parseTag(
    const std::string& path,
    std::set<utils::UniString>& set,
    pt::ptree& root)
{
    for (pt::ptree::value_type& row : root.get_child(path))
    {
        set.insert(utils::UniString(row.second.get_value<std::string>()).toUpperCase().replace(utils::UniCharacter::YO, utils::UniCharacter::YE));
    }
}