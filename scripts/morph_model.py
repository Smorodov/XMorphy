from tensorflow.keras.models import Sequential, Model
from tensorflow.keras.layers import Bidirectional, Conv1D, Flatten
from tensorflow.keras.layers import Dense, Input, Concatenate, Masking
from tensorflow.keras.layers import TimeDistributed, Dropout, BatchNormalization
from tensorflow.keras.utils import to_categorical
from keras.callbacks import EarlyStopping
from keras.preprocessing.sequence import pad_sequences
import numpy as np
from pyxmorphy import UniSPTag, UniMorphTag
import pyxmorphy
import time
from enum import Enum
import fasttext


PARTS_MAPPING = {
    'PREF': 1,
    'ROOT': 2,
    'SUFF': 3,
    'END': 4,
    'LINK': 5,
    'HYPH': 6,
    'POSTFIX': 7,
    'B-SUFF': 8,
    'B-PREF': 9,
    'B-ROOT': 10,
}

LETTERS = {
    'о': 1,
    'е': 2,
    'а': 3,
    'и': 4,
    'н': 5,
    'т': 6,
    'с': 7,
    'р': 8,
    'в': 9,
    'л': 10,
    'к': 11,
    'м': 12,
    'д': 13,
    'п': 14,
    'у': 15,
    'я': 16,
    'ы': 17,
    'ь': 18,
    'г': 19,
    'з': 20,
    'б': 21,
    'ч': 22,
    'й': 23,
    'х': 24,
    'ж': 25,
    'ш': 26,
    'ю': 27,
    'ц': 28,
    'щ': 29,
    'э': 30,
    'ф': 31,
    'ъ': 32,
    'ё': 33,
    '-': 34,
}


VOWELS = {
    'а', 'и', 'е', 'ё', 'о', 'у', 'ы', 'э', 'ю', 'я'
}

CIPH = {
    'о': 0.10983,
    'е': 0.08483,
    'а': 0.07998,
    'и': 0.07367,
    'н': 0.067,
    'т': 0.06318,
    'с': 0.05473,
    'р': 0.04746,
    'в': 0.04533,
    'л': 0.04343,
    'к': 0.03486,
    'м': 0.03203,
    'д': 0.02977,
    'п': 0.02804,
    'у': 0.02615,
    'я': 0.02001,
    'ы': 0.01898,
    'ь': 0.01735,
    'г': 0.01687,
    'з': 0.01641,
    'б': 0.01592,
    'ч': 0.0145,
    'й': 0.01208,
    'х': 0.00966,
    'ж': 0.0094,
    'ш': 0.00718,
    'ю': 0.00639,
    'ц': 0.00486,
    'щ': 0.00361,
    'э': 0.00331,
    'ф': 0.00267,
    'ъ': 0.00037,
    'ё': 0.00013,
    '-': 0.0,
}

SPEECH_PARTS = [
    UniSPTag.X,
    UniSPTag.ADJ,
    UniSPTag.ADV,
    UniSPTag.INTJ,
    UniSPTag.NOUN,
    UniSPTag.PROPN,
    UniSPTag.VERB,
    UniSPTag.ADP,
    UniSPTag.AUX,
    UniSPTag.CONJ,
    UniSPTag.SCONJ,
    UniSPTag.DET,
    UniSPTag.NUM,
    UniSPTag.PART,
    UniSPTag.PRON,
    UniSPTag.PUNCT,
    UniSPTag.H,
    UniSPTag.R,
    UniSPTag.Q,
    UniSPTag.SYM,
]

CASE_TAGS = [
    UniMorphTag.UNKN,
    UniMorphTag.Ins,
    UniMorphTag.Acc,
    UniMorphTag.Nom,
    UniMorphTag.Dat,
    UniMorphTag.Gen,
    UniMorphTag.Loc,
    UniMorphTag.Voc,
]

NUMBER_TAGS = [
    UniMorphTag.UNKN,
    UniMorphTag.Sing,
    UniMorphTag.Plur,
]

GENDER_TAGS = [
    UniMorphTag.UNKN,
    UniMorphTag.Masc,
    UniMorphTag.Fem,
    UniMorphTag.Neut,
]

TENSE_TAGS = [
    UniMorphTag.UNKN,
    UniMorphTag.Fut,
    UniMorphTag.Past,
    UniMorphTag.Pres,
    UniMorphTag.Notpast,
]


MASK_VALUE = 0

speech_part_len = len(SPEECH_PARTS)
speech_part_mapping = {str(s): num for num, s in enumerate(SPEECH_PARTS)}

case_len = len(CASE_TAGS)
case_mapping = {str(s): num for num, s in enumerate(CASE_TAGS)}

number_len = len(NUMBER_TAGS)
number_mapping = {str(s): num for num, s in enumerate(NUMBER_TAGS)}

gender_len = len(GENDER_TAGS)
gender_mapping = {str(s): num for num, s in enumerate(GENDER_TAGS)}

tense_len = len(TENSE_TAGS)
tense_mapping = {str(s): num for num, s in enumerate(TENSE_TAGS)}


class MorphemeLabel(Enum):
    PREF = 'PREF'
    ROOT = 'ROOT'
    SUFF = 'SUFF'
    END = 'END'
    LINK = 'LINK'
    HYPH = 'HYPH'
    POSTFIX = 'POSTFIX'
    NONE = None


class Morpheme(object):
    def __init__(self, part_text, label, begin_pos):
        self.part_text = part_text
        self.length = len(part_text)
        self.begin_pos = begin_pos
        self.label = label
        self.end_pos = self.begin_pos + self.length

    def __len__(self):
        return self.length

    def get_labels(self):
        if self.length == 1:
            return ['S-' + self.label.value]
        result = ['B-' + self.label.value]
        result += ['M-' + self.label.value for _ in self.part_text[1:-1]]
        result += ['E-' + self.label.value]
        return result

    def get_simple_labels(self):
        if self.label == MorphemeLabel.SUFF or self.label == MorphemeLabel.PREF or self.label== MorphemeLabel.ROOT:
            result = ['B-' + self.label.value]
            if self.length > 1:
                result += [self.label.value for _ in self.part_text[1:]]
            return result
        else:
            return [self.label.value] * self.length

    def __str__(self):
        return self.part_text + ':' + self.label.value

    @property
    def unlabeled(self):
        return not self.label.value


class Word(object):
    def __init__(self, morphemes=[]):
        self.morphemes = morphemes

    def append_morpheme(self, morpheme):
        self.morphemes.append(morpheme)

    def get_word(self):
        return ''.join([morpheme.part_text for morpheme in self.morphemes])

    def parts_count(self):
        return len(self.morphemes)

    def suffix_count(self):
        return len([morpheme for morpheme in self.morphemes if morpheme.label == MorphemeLabel.SUFFIX])

    def get_labels(self):
        result = []
        for morpheme in self.morphemes:
            result += morpheme.get_labels()
        return result

    def get_simple_labels(self):
        result = []
        for morpheme in self.morphemes:
            result += morpheme.get_simple_labels()
        return result

    def __str__(self):
        return '/'.join([str(morpheme) for morpheme in self.morphemes])

    def __len__(self):
        return sum(len(m) for m in self.morphemes)

    @property
    def unlabeled(self):
        return all(p.unlabeled for p in self.morphemes)


def parse_morpheme(str_repr, position):
    text, label = str_repr.split(':')
    return Morpheme(text, MorphemeLabel[label], position)


def parse_word(str_repr):
    _, word_parts = str_repr.split('\t')
    parts = word_parts.split('/')
    morphemes = []
    global_index = 0
    for part in parts:
        morphemes.append(parse_morpheme(part, global_index))
        global_index += len(part)
    return Word(morphemes)


def measure_quality(predicted_targets, targets, words):
    TP, FP, FN, equal, total = 0, 0, 0, 0, 0
    SE = ['{}-{}'.format(x, y) for x in "SE" for y in ["ROOT", "PREF", "SUFF", "END", "LINK", "None"]]
    corr_words = 0
    for corr, pred, word in zip(targets, predicted_targets, words):
        corr_len = len(corr)
        pred_len = len(pred)
        boundaries = [i for i in range(corr_len) if corr[i] in SE]
        pred_boundaries = [i for i in range(pred_len) if pred[i] in SE]
        common = [x for x in boundaries if x in pred_boundaries]
        TP += len(common)
        FN += len(boundaries) - len(common)
        FP += len(pred_boundaries) - len(common)
        equal += sum(int(x==y) for x, y in zip(corr, pred))
        total += len(corr)
        corr_words += (corr == pred)
        if corr != pred:
            print("Error in word '{}':\n correct:".format(word.get_word()), corr, '\n!=\n wrong:', pred)

    metrics = ["Precision", "Recall", "F1", "Accuracy", "Word accuracy"]
    results = [TP / (TP+FP), TP / (TP+FN), TP / (TP + 0.5*(FP+FN)),
               equal / total, corr_words / len(targets)]
    return list(zip(metrics, results))


#def _get_word_info(word, analyzer):
#    analyzed = analyzer.analyze(word, True, False)[0]
#    most_probable = max(analyzed.infos, key=lambda x: x.probability)
#
#    #normal_form = most_probable.normal_form
#    pos = to_categorical(speech_part_mapping[str(most_probable.sp)], num_classes=speech_part_len)
#    case = to_categorical(case_mapping[str(most_probable.tag.get_case())], num_classes=case_len)
#    gender = to_categorical(gender_mapping[str(most_probable.tag.get_gender())], num_classes=gender_len)
#    number = to_categorical(number_mapping[str(most_probable.tag.get_number())], num_classes=number_len)
#    tense = to_categorical(tense_mapping[str(most_probable.tag.get_tense())], num_classes=tense_len)
#    #return pos.tolist() + case.tolist() + gender.tolist() + number.tolist() + tense.tolist() + list(embedder.get_word_vector(word))
#    return []


def _get_parse_repr(word):
    features = []
    word_text = word.get_word()
    #word_morph_info = _get_word_info(word_text, analyzer)
    #print(word, "MORPH INFO:", word_morph_info)
    for index, letter in enumerate(word_text):
        letter_features = [] #word_morph_info.copy()
        vovelty = 0
        if letter in VOWELS:
            vovelty = 1
        letter_features.append(vovelty)
        letter_features += to_categorical(LETTERS[letter], num_classes=len(LETTERS) + 1).tolist()
        #print("LETTER:", letter)
        #print("LETTER FEATURES", to_categorical(LETTERS[letter], num_classes=len(LETTERS) + 1).tolist())
        features.append(letter_features)

    X = np.array(features, dtype=np.int8)
    Y = np.array([to_categorical(PARTS_MAPPING[label], num_classes=len(PARTS_MAPPING) + 1) for label in word.get_simple_labels()])
    #print("SIMPLE LABELS:", word.get_simple_labels())
    #print("Y", Y)
    #exit(1)
    return X, Y


def _pad_sequences(Xs, Ys, max_len):
    newXs = pad_sequences(Xs, padding='post', dtype=np.int8, maxlen=max_len, value=MASK_VALUE)
    newYs = pad_sequences(Ys, padding='post', maxlen=max_len, value=MASK_VALUE)
    return newXs, newYs


#analyzer = pyxmorphy.MorphAnalyzer()


def _prepare_words(words, max_len):
    result_x, result_y = [], []
    print("Preparing words")
    for i, word in enumerate(words):
        word_x, word_answer = _get_parse_repr(word)
        result_x.append(word_x)
        result_y.append(word_answer)
        if i % 1000 == 0:
            print("Prepared", i)

    return _pad_sequences(result_x, result_y, max_len)


class MorphemModel(object):
    def __init__(self, dropout, layers, models_number, epochs, validation_split, window_sizes, max_len):
        self.dropout = dropout
        self.layers = layers
        self.models_number = models_number
        self.epochs = epochs
        self.validation_split = validation_split
        self.window_sizes = window_sizes
        self.activation = "softmax"
        self.optimizer = "adam"
        self.models = []
        self.max_len = max_len

    def _transform_classification(self, parse):
        parts = []
        current_part = [parse[0]]
        for num, letter in enumerate(parse[1:]):
            index = num + 1
            if letter == 'SUFF' and parse[index - 1] == 'B-SUFF':
                current_part.append(letter)
            elif letter == 'PREF' and parse[index - 1] == 'B-PREF':
                current_part.append(letter)
            elif letter == 'ROOT' and parse[index - 1] == 'B-ROOT':
                current_part.append(letter)
            elif letter != parse[index - 1] or letter.startswith('B-'):
                parts.append(current_part)
                current_part = [letter]
            else:
                current_part.append(letter)
        if current_part:
            parts.append(current_part)

        for part in parts:
            if part[0] == 'B-PREF':
                part[0] = 'PREF'
            if part[0] == 'B-SUFF':
                part[0] = 'SUFF'
            if part[0] == 'B-ROOT':
                part[0] = 'ROOT'
            if len(part) == 1:
                part[0] = 'S-' + part[0]
            else:
                part[0] = 'B-' + part[0]
                part[-1] = 'E-' + part[-1]
                for num, letter in enumerate(part[1:-1]):
                    part[num+1] = 'M-' + letter
        result = []
        for part in parts:
            result += part
        return result

    def _build_model(self, input_maxlen):
        inp = Input(shape=(input_maxlen, len(LETTERS) + 1 + 1))
        inputs = [inp]
        do = None
       #inp = BatchNormalization()(inp)

        conv_outputs = []
        for drop, units, window_size in zip(self.dropout, self.layers, self.window_sizes):
            conv = Conv1D(units, window_size, activation='relu', padding="same")(inp)
            #norm = BatchNormalization()(conv)
            do = Dropout(drop)(conv)
            inp = do
            conv_outputs.append(do)

        concat = Concatenate(name="conv_output")(conv_outputs)
        #pre_last_output = TimeDistributed(
        #    Dense(64, activation="relu"),
        #    name="pre_output")(concat)

        outputs = [TimeDistributed(
            Dense(len(PARTS_MAPPING) + 1, activation=self.activation))(concat)]
        self.models.append(Model(inputs, outputs=outputs))
        self.models[-1].compile(loss='categorical_crossentropy',
                                optimizer=self.optimizer, metrics=['acc'])

        print(self.models[-1].summary())

    def train(self, words):
        (x, y,) = _prepare_words(words, self.max_len)
        for i in range(self.models_number):
            self._build_model(self.max_len)
        es = EarlyStopping(monitor='val_acc', patience=10, verbose=1)
        self.models[-1].fit(x, y, epochs=self.epochs, verbose=2,
                  callbacks=[es], validation_split=self.validation_split)
        self.models[-1].save("keras_morphem_model_{}.h5".format(int(time.time())))

    def classify(self, words):
        print("Total models:", len(self.models))
        (x, _,) = _prepare_words(words, self.max_len)
        pred = self.models[-1].predict(x)
        pred_class = pred.argmax(axis=-1)
        reverse_mapping = {v: k for k, v in PARTS_MAPPING.items()}
        result = []
        corrected = 0
        for i, word in enumerate(words):
            cutted_prediction = pred_class[i][:len(word.get_word())]
            raw_parse = [reverse_mapping[int(num)] for num in cutted_prediction]
            raw_probs = pred[i][:len(word.get_word())]
            if raw_probs != raw_parse:
                print("Word:", word.get_word())
                print("Raw:", raw_parse)
                corrected += 1
            parse = self._transform_classification(raw_parse)
            result.append(parse)
        print("Totally corrected:", corrected)
        return result


if __name__ == "__main__":
    train_part = []
    counter = 0
    max_len = 0
    with open('./datasets/tikhonov.train', 'r') as data:
        for num, line in enumerate(data):
            counter += 1
            train_part.append(parse_word(line.strip()))
            max_len = max(max_len, len(train_part[-1]))
            if counter % 1000 == 0:
                print("Loaded", counter, "train words")

    test_part = []
    with open('./datasets/tikhonov.test', 'r') as data:
        for num, line in enumerate(data):
            counter += 1
            test_part.append(parse_word(line.strip()))
            max_len = max(max_len, len(train_part[-1]))
            if counter % 1000 == 0:
                print("Loaded", counter, "test words")

    print("MAXLEN", max_len)
    model = MorphemModel([0.4, 0.4, 0.4], [512, 512, 512], 1, 100, 0.1, [5, 5, 5], max_len)
    train_time = model.train(train_part)
    result = model.classify(test_part)

    print(measure_quality(result, [w.get_labels() for w in test_part], test_part))