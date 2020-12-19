//B. Заборчик для кроликов
//        Ограничение времени	2 секунды
//        Ограничение памяти	256Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//        Арсений продолжает спать. Теперь ему снится кроличья ферма на планете Ка-Пэкс.
//        Правда, ферма у него так себе — одни кроличьи норы в пустом поле.
//        Чтобы её хоть как-то облагородить, Арсений решил поставить забор так,
//        чтобы все норки оказались внутри огражденной территории,
//        а досок он потратил как можно меньше.
//        Помогите Арсению найти длину забора, чтобы он мог уже заказывать стройматериалы!
//
//Формат ввода
// В первой строке вводится число N (3 ≤ N ≤ 10^5).
// Следующие N строк содержат пары целых чисел x и y (-10^9 ≤ x, y ≤ 10^9)
// — координаты кроличьих нор.
//
//Формат вывода
// Одно вещественное число — длину забора — с максимально возможной точностью.

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "geometry2D.cpp"

template <typename T>
double calcChainLen(const std::vector<Point<T>> & chain) {
    double result = 0;

    auto p1_ptr = chain.begin();

    for (auto p2_ptr = chain.begin() + 1; p2_ptr != chain.end(); ++p2_ptr) {
        result += (*p1_ptr - *p2_ptr).len();
        p1_ptr = p2_ptr;
    }

    // add last edge (chain[n-1]; chain[0])
    result += (*chain.begin() - *(chain.end()-1)).len();
    return result;
}

int main() {
    using T = int64_t;

    size_t N;
    std::cin >> N;

    std::vector<Point<T>> points;
    points.reserve(N);

    for (size_t i = 0; i < N; ++i) {
        Point<T> p{};
        std::cin >> p;
        points.emplace_back(p);
    }

    ConvexHull<std::vector<Point<T>>, T> chc(points.begin(), points.end());
    auto convex_hull = chc.createConvexHull();

    std::cout << std::setprecision(10) << calcChainLen(convex_hull.getVertices());
}

//13
//0 2
//1 1
//1 2
//1 3
//2 0
//2 1
//2 2
//2 3
//2 4
//3 1
//3 2
//3 3
//4 2

//wrong(16)
//17
//3 3
//4 2
//2 0
//2 1
//1 1
//0 2
//3 1
//3 2
//1 2
//1 3
//2 3
//2 4
//2 2
//0 0
//4 0
//0 4
//4 4

//wrong(12)
//13
//0 0
//0 1
//0 2
//1 2
//1 1
//2 0
//2 1
//2 2
//3 0
//3 1
//4 0
//4 2
//4 1

//wrong(12)
//13
//0 0
//0 1
//0 2
//1 2
//1 1
//2 0
//2 1
//2 2
//0 -1
//0 -2
//1 -1
//2 -1
//2 -2

//wrong
//10
//0 0
//0 1
//0 2
//1 2
//1 1
//2 0
//2 1
//2 2
//0 -1
//2 -1

//wrong(10)
//8
//0 0
//0 1
//0 2
//0 3
//1 2
//1 1
//2 0
//2 3


//16
//0 0
//1 0
//2 0
//2 1
//2 2
//1 2
//0 2
//0 1
//0 0
//1 0
//2 0
//2 1
//2 2
//1 2
//0 2
//0 1