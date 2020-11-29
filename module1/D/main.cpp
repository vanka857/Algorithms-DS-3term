#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <functional>

class SuffixArray {
private:
    const size_t ALPHABET_SIZE = 256;

    // The order of fields is important (member initializing list)
    std::string text_;
    size_t size_;
    std::vector<int> suffix_array_;
    std::vector<int> equivalence_classes_;
    std::vector<int> LCP_;
    size_t n_of_classes_ = 0;

    void fillSuffixArray();

    void sortByFirst2PowKSymbols(size_t k);
    void buildLCPUsingKasai();

public:
    explicit SuffixArray(std::string text);
    size_t calcDifferentSubstrings();
};

// size_ = text_.size() + 1 because in constructor body we increase text_, but it executed finally
SuffixArray::SuffixArray(std::string text) : text_(std::move(text)), size_(text_.size() + 1), suffix_array_(size_), equivalence_classes_(size_){
    text_ += '$';

    fillSuffixArray();
}

std::vector<int> count(const size_t & alphabet_size, const std::function<int (size_t i)>& getI, const size_t & src_size) {
    // функция расчёта позиции элементов, получаемых из getI(size_t i) в сортировке подсчётом

    std::vector<int> counter(alphabet_size, 0);

    for (size_t i = 0; i < src_size; ++i) {
        ++counter[getI(i)];
    }
    for (size_t i = 1; i < alphabet_size; ++i) {
        counter[i] += counter[i - 1];
    }

    return std::move(counter);
}

void SuffixArray::fillSuffixArray() {
    // Сначала отсортируем подстроки по 0му символу (нумерация с нуля)

    // Получаем позиции символов из text и делаем сортировку подсчётом, точнее записываем,
    // какая строка будет стоять на i-ой позиции в suffix_array
    auto startsWith = count(ALPHABET_SIZE, [this](size_t i){ return text_[i]; }, size_);
    for (size_t i = 0; i < size_; ++i) {
        --startsWith[text_[i]];

        suffix_array_[startsWith[text_[i]]] = i;
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

    for (size_t k = 1; k <= size_; k <<= 1u) {

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

    // Эта функция сортирует подстроки text_[2^(k - 1) ... 2^k - 1] (нумерация с нуля) и записывает результат в suffix_array_,
    // используя уже отсортированные строки text_[0 ... 2^(k - 1) - 1]

    std::vector<int> suffix_array_1(size_);
    std::vector<int> equivalence_classes_1(size_);

    // "закольцовываем" подстроки
    for (size_t i = 0; i < size_; ++i) {
        suffix_array_1[i] = (suffix_array_[i] - k + size_) % size_;
    }

    // Получаем позиции начала классов эквивалентностей подстрок из suffix_array_1
    // и сортируем подстроки из suffix_array_1 подсчётом (записываем в suffix_array_).
    auto startsWith = count(n_of_classes_, [this, suffix_array_1](size_t i){ return equivalence_classes_[suffix_array_1[i]];}, size_);
    for (size_t i = size_; i > 0; --i) {
        auto & j = startsWith[equivalence_classes_[suffix_array_1[i - 1]]];
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

size_t SuffixArray::calcDifferentSubstrings() {
    if (LCP_.empty()) {
        buildLCPUsingKasai();
    }

    // Вспомним, что мы добавили к нашей строке '$', и вычтем 1 из размера строки для удобства расчета
    --size_;

    // Если бы все подстроки априори бы ли бы различными, то их число просто выражалось бы такой формулой
    size_t result = (size_ + 1) * size_ / 2;

    // Но так как подстроки могут быть префиксами друг друга, учтем количество повторений подстрок,
    // то есть количество совпадающих символов для каждой подстроки.
    // Для этого вычтем из результата сумму всего LCP
    for (size_t i = 0; i < size_; ++i) {
        result -= LCP_[i];
    }

    return result;
}


int main() {
    std::string text;
    std::cin >> text;

    SuffixArray suffix_array(text);
    std::cout << suffix_array.calcDifferentSubstrings();

    return 0;
}
