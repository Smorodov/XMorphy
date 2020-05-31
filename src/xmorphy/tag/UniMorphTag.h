#pragma once
#include "ITag.h"
namespace X
{
struct UniMorphTag : public ITag
{
private:
    UniMorphTag(uint64_t val);
    static const std::vector<UniMorphTag> inner_runner;

public:
    static const UniMorphTag UNKN;
    // Gender
    static const UniMorphTag Masc;
    static const UniMorphTag Fem;
    static const UniMorphTag Neut;
    // Animacy
    static const UniMorphTag Anim;
    static const UniMorphTag Inan;
    // Number
    static const UniMorphTag Sing;
    static const UniMorphTag Plur;
    // Case
    static const UniMorphTag Ins;
    static const UniMorphTag Acc;
    static const UniMorphTag Nom;
    static const UniMorphTag Dat;
    static const UniMorphTag Gen;
    static const UniMorphTag Loc;
    static const UniMorphTag Voc;
    // Degree of comparison
    static const UniMorphTag Cmp;
    static const UniMorphTag Sup;
    static const UniMorphTag Pos;
    // VerbForm
    static const UniMorphTag Fin;
    static const UniMorphTag Inf;
    static const UniMorphTag Conv;
    // Mood
    static const UniMorphTag Imp;
    static const UniMorphTag Ind;
    // Person
    static const UniMorphTag _1;
    static const UniMorphTag _2;
    static const UniMorphTag _3;
    // Tense
    static const UniMorphTag Fut;
    static const UniMorphTag Past;
    static const UniMorphTag Pres;
    static const UniMorphTag Notpast;
    // Variant
    static const UniMorphTag Brev;
    // Voice
    static const UniMorphTag Act;
    static const UniMorphTag Pass;
    static const UniMorphTag Mid;
    //Form
    static const UniMorphTag Digit;

    static std::vector<UniMorphTag>::const_iterator begin() { return inner_runner.begin(); }

    static std::vector<UniMorphTag>::const_iterator end() { return inner_runner.end(); }

    static constexpr size_t genderSize() { return 3; }

    static constexpr size_t numberSize() { return 2; }

    static constexpr size_t caseSize() { return 7; }

    static constexpr size_t tenseSize() { return 4; }

    static constexpr size_t animacySize() { return 2; }

    static UniMorphTag getGen(size_t index)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Masc) - inner_runner.begin();
        return inner_runner[start + index];
    }

    static UniMorphTag getNum(size_t index)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Sing) - inner_runner.begin();
        return inner_runner[start + index];
    }

    static UniMorphTag getCase(size_t index)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Ins) - inner_runner.begin();
        return inner_runner[start + index];
    }

    static UniMorphTag getTense(size_t index)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Fut) - inner_runner.begin();
        return inner_runner[start + index];
    }

    static UniMorphTag getAnimacy(size_t index)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Anim) - inner_runner.begin();
        return inner_runner[start + index];
    }

    static size_t getGen(UniMorphTag t)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Masc) - inner_runner.begin();
        size_t result = (std::find(inner_runner.begin(), inner_runner.end(), t) - inner_runner.begin()) - start;
        return result;
    }

    static size_t getNum(UniMorphTag t)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Sing) - inner_runner.begin();
        size_t result = (std::find(inner_runner.begin(), inner_runner.end(), t) - inner_runner.begin()) - start;
        return result;
    }

    static size_t getCase(UniMorphTag t)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Ins) - inner_runner.begin();
        size_t result = (std::find(inner_runner.begin(), inner_runner.end(), t) - inner_runner.begin()) - start;
        return result;
    }

    static size_t getTense(UniMorphTag t)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Fut) - inner_runner.begin();
        size_t result = (std::find(inner_runner.begin(), inner_runner.end(), t) - inner_runner.begin()) - start;
        return result;
    }

    static size_t getAnimacy(UniMorphTag t)
    {
        size_t start = std::find(inner_runner.begin(), inner_runner.end(), UniMorphTag::Anim) - inner_runner.begin();
        size_t result = (std::find(inner_runner.begin(), inner_runner.end(), t) - inner_runner.begin()) - start;
        return result;
    }

    UniMorphTag intersect(const ITag & o) const
    {
        uint64_t newValue = this->value & (uint64_t)o;
        return UniMorphTag(newValue);
    }

    UniMorphTag operator|(const UniMorphTag & o) const;
    bool operator==(const UniMorphTag & other) const { return value == other.value && name_map == other.name_map; }
    bool operator!=(const UniMorphTag & other) const { return !this->operator==(other); }
    bool operator<(const UniMorphTag & other) const { return value < other.value; }
    bool operator>(const UniMorphTag & other) const { return value > other.value; }

    void setGender(const UniMorphTag & gender);
    void setNumber(const UniMorphTag & number);
    void setCase(const UniMorphTag & cas);
    void setTense(const UniMorphTag & tense);
    void setAnimacy(const UniMorphTag & anim);
    UniMorphTag getGender() const;
    UniMorphTag getNumber() const;
    UniMorphTag getCase() const;
    UniMorphTag getTense() const;
    UniMorphTag getAnimacy() const;

    UniMorphTag(const std::string & val);
    UniMorphTag();
};
}