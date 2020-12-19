//C. Жестокая игра
//Ограничение времени	2 секунды
//        Ограничение памяти	64Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//        Штирлиц и Мюллер стреляют по очереди. В очереди n человек, стоящих друг за другом.
//        Каждым выстрелом убивается один из стоящих. Кроме того, если у кого-то из стоящих в
//        очереди убиты все его соседи, то этот человек в ужасе убегает. Проигрывает тот, кто не может ходить.
//        Первым стреляет Штирлиц. Требуется определить, кто выиграет при оптимальной игре обеих сторон,
//        и если победителем будет Штирлиц, то найти все возможные первые ходы, ведущие к его победе.
//
//Формат ввода
//  Входной файл содержит единственное число n (2 ≤ n ≤ 5 000) — количество человек в очереди.
//
//Формат вывода
//  Если выигрывает Мюллер, выходной файл должен состоять из единственного слова Mueller.
//  Иначе в первой строке необходимо вывести слово Schtirlitz, а в последующих строках — номера людей в очереди,
//  которых мог бы первым ходом убить Штирлиц для достижения своей победы.
//  Номера необходимо выводить в порядке возрастания.
//
//Пример 1
//3
//
//Schtirlitz
//2
//
//Пример 2
//4
//
//Mueller
//
//Пример 3
//5
//
//Schtirlitz
//1
//3
//5

#include <iostream>
#include <vector>
#include <set>

// Класс, реализующий вычисление игрока, выигрывающего в игре,
// которыую можно свести к игре Ним по теореме Шпрага-Гранди, а также возвраща
class SG {

    std::vector<ssize_t> calculated;
    std::vector<ssize_t> feedback;

    bool first_win;

public:
    explicit SG(size_t n) : calculated(n + 1, -1){
        first_win = spragueGrundy(n, &feedback);
    }
    [[nodiscard]] std::vector<ssize_t> goodChoises() const {
        return feedback;
    }
    [[nodiscard]] bool is_first_win() const {
        return first_win;
    }

private:
    // Функция, вычисляющее значение функции Шпрага-Гранди от игры Ним с количеством камешек n.
    // При необходимости заполняет массив feedback выиигрышными в этой игре ходами
    // (позициями, убирая камешки из которых мы разделим эту игру на две независимых)
    size_t spragueGrundy(size_t n, std::vector<ssize_t> * feedback_ptr = nullptr) {

        if (n == 0) {
            return 0;
        }

        if (calculated[n] != -1) {
            // используем динамику для избежания повторных рассчетов
            return calculated[n];
        }

        std::vector<size_t> values;
        for(size_t i = 0; i < n; ++i){
            // Если убьем второго слева, то в левой игре останется один человек, значит он сразу убежит и результат игры будет 0
            size_t l = i == 1 ? 0 : spragueGrundy(i);

            // Если убьем второго справа, то в правой игре останется один человек, значит он сразу убежит и результат игры будет 0
            size_t r = n - i - 1 == 1 ? 0 : spragueGrundy(n - i - 1);

            if (feedback_ptr != nullptr && ((l ^ r) == 0)) {
                feedback.push_back(i);
            }

            // Согласно решению игры Ним Чарлза Бутона, решением игры Ним будет XOR решений двух независимых под-игр, составляющих нашу игру Ним.
            values.push_back(l ^ r);
        }

        return calculated[n] = mex(values);
    }
    // Функция, возвращающая наименьшее натуральное (с нулем) число, не встретившееся в массиве data
    static size_t mex(const std::vector<size_t> & data) {
        if (data.empty()) {
            return 0;
        }

        std::set<size_t> st(data.begin(), data.end());

        for (size_t i = 0; ; ++i) {
            if(st.count(i) == 0) {
                return i;
            }
        }
    }
};


int main() {
    size_t n;
    std::cin >> n;

    SG sg(n);

    if (sg.is_first_win()) {
        std::cout << "Schtirlitz\n";
        for (const auto & i : sg.goodChoises()) {
            std::cout << i + 1 << "\n";
        }
    }
    else {
        std::cout << "Mueller\n";
    }

    return 0;
}
