#pragma once

#include <memory>
#include <xmorphy/ml/KerasModel.h>
#include <xmorphy/morph/WordForm.h>

namespace X
{
class MorphemicSplitter
{
private:
    std::unique_ptr<KerasModel> model;
    size_t sequence_size;


public:
    MorphemicSplitter(std::istream & model_stream_, size_t sequence_size_)
        : model(std::make_unique<KerasModel>(model_stream_)), sequence_size(sequence_size_)
    {
    }

    MorphemicSplitter();

    void split(WordFormPtr forms) const;
    std::vector<PhemTag> split(const utils::UniString & word, UniSPTag sp) const;

    std::vector<PhemTag> split(const utils::UniString & word, UniSPTag sp, const utils::UniString normal_form, const std::vector<PhemTag> & normal_form_parse) const;
};

}
