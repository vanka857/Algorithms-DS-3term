#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <optional>

class SuffixArray {
private:
    const size_t ALPHABET_SIZE = 256;
    size_t size_;
    std::string text_;
    std::vector<int> suffix_array_;
    std::vector<int> equivalence_classes_;
    std::vector<int> LCP_;
    size_t n_of_classes_ = 0;

    size_t n_of_strings_ = 1;
    std::vector<size_t> length_of_strings_;

public:
    explicit SuffixArray(std::string text);
    SuffixArray(const std::string & text1, const std::string & text2) : SuffixArray(text1 + '$' + text2) {
        n_of_strings_ = 2;
        length_of_strings_.push_back(text1.size() + 1);
        length_of_strings_.push_back(text2.size() + 1);
    }

private:
    void fillSuffixArray();

    void sortByFirst2PowKSymbols(size_t k);
    void buildLCPUsingKasai();

public:
    [[nodiscard]] std::optional<std::string> orderedCommonSubstring(size_t order_len) const;
};

SuffixArray::SuffixArray(std::string text) : text_(std::move(text)){
    text_ += '#';
    size_ = text_.size();
    suffix_array_.resize(size_);
    equivalence_classes_.resize(size_);

    fillSuffixArray();
    buildLCPUsingKasai();
}

void SuffixArray::fillSuffixArray() {
    // Сначала отсортируем подстроки по 0му символу (нумерация с нуля)

    std::vector<int> counter(ALPHABET_SIZE, 0); //  счетчик вхождений сортировки подсчётом
    for (auto ch : text_) {
        ++counter[ch];
    }
    for (size_t i = 1; i < ALPHABET_SIZE; ++i) {
        counter[i] += counter[i - 1];
    }
    for (size_t i = 0; i < size_; ++i) {
        --counter[text_[i]];

        // сортируем подстроки по возрастанию 0го символа? точнее записываем,
        // какая строка будет стоять на i-ой позиции в suffix_array
        suffix_array_[counter[text_[i]]] = i;
    }

    // Потом заполним массив классов эквивалентностей

    n_of_classes_ = 1;
    for (size_t i = 1; i < size_; ++i) {
        if (text_[suffix_array_[i]] != text_[suffix_array_[i - 1]]) {
            ++n_of_classes_;
        }

        equivalence_classes_[suffix_array_[i]] = n_of_classes_ - 1;
    }

    // Отсортируем в цикле подстроки длинной 2^k > size_

    for (size_t i = 0, k = 1; ; ++i, k = 1u << i) {

        // Если на очередном шаге сортировки получилось, что все подстроки находятся в разных классах эквивалентности,
        // Значит от сортировки внутри одного класса экв-ти (если учитывать следующие 2^(k - 1) символов)
        // порядок не изменится (Очевидно, т.к. в каждом классе экв-ти осталось по одной подстроке).
        //
        // Заканчиваем, если все наши подстроки уникальны и отсортированы (описано выше) или 2^k > size_
        if (n_of_classes_ == size_ || k > size_) {
            break;
        }

        sortByFirst2PowKSymbols(k);
    }
}
void SuffixArray::sortByFirst2PowKSymbols(size_t k) {
    // На этом шаге сортировки выходным результатом будут отсортированные строки длины 2^k символов

    std::vector<int> suffix_array_1(size_);
    std::vector<int> equivalence_classes_1(size_);

    for (size_t i = 0; i < size_; ++i) {
        // сортируем подстроки text_[2^(k - 1) ... 2^k - 1] (нумерация с нуля) и записываем результат в suffix_array_1
        // для этого используем уже отсортированные строки text_[0 ... 2^(k - 1) - 1]
        // и при необходимости "закольцовываем" подстроки
        suffix_array_1[i] = (suffix_array_[i] - k + size_) % size_;
    }
    // сортируем подсчётом
    std::vector<int> counter(n_of_classes_, 0);
    for (size_t i = 0; i < size_; ++i) {
        ++counter[equivalence_classes_[suffix_array_1[i]]];
    }
    for (size_t i = 1; i < n_of_classes_; ++i) {
        counter[i] += counter[i - 1];
    }
    for (size_t i = size_; i > 0; --i) {
        auto & j = counter[equivalence_classes_[suffix_array_1[i - 1]]];
        --j;
        suffix_array_[j] = suffix_array_1[i - 1];
    }
    equivalence_classes_1[suffix_array_[0]] = 0;

    n_of_classes_ = 1;

    // обновляем классы эквивалентности (записываем новые в equivalence_classes_1), если:
    // 1) при различии классов экв. у первых половинок подстрок длинны 2^k.
    //    То есть при различии классов эквивалентности у подстрок text_[0 ... 2^(k - 1) - 1]
    // 2) при различии классов экв. у вторых половинок подстрок длинны 2^k.
    //    То есть при различии классов эквивалентности у подстрок text_[2^(k - 1) ... 2^k - 1]

    for (size_t i = 1; i < size_; ++i) {
        size_t right_middle = (suffix_array_[i] + k) % size_;
        size_t left_middle = (suffix_array_[i - 1] + k) % size_;

        if (equivalence_classes_[suffix_array_[i]] != equivalence_classes_[suffix_array_[i - 1]] ||
            equivalence_classes_[left_middle] != equivalence_classes_[right_middle]) {
            ++n_of_classes_;
        }

        equivalence_classes_1[suffix_array_[i]] = n_of_classes_ - 1;
    }

    // Сохраняем новый классы эквивалентности подстрок

    equivalence_classes_ = std::move(equivalence_classes_1);
}
void SuffixArray::buildLCPUsingKasai() {
    // Алгоритм Касаи и его друзей построения LCP
    LCP_.resize(size_ - 1);
    std::vector<int> position(size_);

    // строим симметричный суффиксному массиву массив для удобства
    for (size_t i = 0; i < size_; ++i) {
        position[suffix_array_[i]] = i;
    }

    size_t n_of_similar_symbols = 0;
    for (size_t i = 0; i < size_; ++i) {
        if (n_of_similar_symbols > 0) {
            // С каждой итерацией цикла количество совпадающих символов уменьшается как минимум на единицу
            --n_of_similar_symbols;
        }

        // Так как у нас закольцованые подстроки, нужно пресечь поиск
        // повторных символов в подстроке $... (последней подстроке)
        if (position[i] == size_ - 1) {
            // Соответственно, сбрасываем счетчик повторяющихся символов
            n_of_similar_symbols = 0;
            continue;
        }
            // Если рассматриваем сейчас не последнюю строку ($...)
        else {
            // То сравним текущую строку со следующей по порядку в суффиксном массиве
            size_t j = suffix_array_[position[i] + 1];

            // И найдём в них количество повторяющихся символов
            while (std::max(i + n_of_similar_symbols, j + n_of_similar_symbols) < size_
                   && text_[i + n_of_similar_symbols] == text_[j + n_of_similar_symbols]) {
                ++n_of_similar_symbols;
            }

            LCP_[position[i]] = n_of_similar_symbols;
        }
    }

}

std::optional<std::string> SuffixArray::orderedCommonSubstring(size_t order_len) const {
    if (n_of_strings_ != 2) {
        return { };
    }

    size_t index_of_string_splitter = length_of_strings_[0] - 1; // место, с которого начинается вторая строка
    size_t n_of_matched_substring = 0; // число совпадающих подстрок

    // Мы пропускаем первые две строки (начинаются со служебных символов '$' и '#'), т.к. их рассмотрение ничего не даёт

    // Изначально min_LCP = 0, т.к. для корректности алгоритма min_LCP равен нулю на первых, "служебных", строках.
    size_t min_LCP = 0;

    for (size_t i = 2; i < size_ - 1; ++i) {

        if (LCP_[i] < min_LCP) {
            min_LCP = LCP_[i];
        }

        // если i-я и i+1-я подстроки в суффиксном массиве принадлежат разным строкам text1 и text2,
        // то есть лежат по разные стороны от index_of_string_splitter в строке text_
        if (suffix_array_[i] > index_of_string_splitter ^ suffix_array_[i + 1] > index_of_string_splitter) {
            // Тут понятно
            n_of_matched_substring += LCP_[i];

            // Нам нужно вычесть уже учтенные подстроки. Их число будет равно LCP двух последних подстрок,
            // на которых мы заходили в этот if, то есть подстрок из одной строки,
            // между которыми будут подстроки только из другой строки.
            // LCP двух строк будет равно минимуму массива LCP на отрезке, включая эти подстроки
            n_of_matched_substring -= min_LCP;

            // Насильно обновляем минимум
            min_LCP = LCP_[i];
        }

        // Заметим, что находить общие подстроки мы будем в лексикогрфическом порядке, т.к. мы ищем их в
        // суффиксном массиве, который уже отсортирован

        // Если это order_len-я по счету общая подстрока, выходим
        if (n_of_matched_substring >= order_len) {
            // Может оказаться так, что на очередном шаге мы нашли больше, чем нужно общих строк (n_of_matched_substring > order_len)
            // Тогда выведем строку text_[j ... j + LCP_[i] - перескок], 
            // где перескок - то, насколько больше общих строк мы нашли на i-м шаге. Перескок = n_of_matched_substring - order_len
            // А LCP_[i] - количество общих символов текущей и следующей строки (оно может буть больше, чем нам нужно!)
            return {std::string(text_.begin() + suffix_array_[i],
                    text_.begin() + suffix_array_[i] + LCP_[i] - (n_of_matched_substring - order_len))};
        }
    }

    return { };
}


int main() {
    std::string text1, text2;
    size_t k;

    std::cin >> text1 >> text2 >> k;

    SuffixArray suffix_array(text1, text2);
    std::cout << suffix_array.orderedCommonSubstring(k).value_or("-1");

    return 0;
}
