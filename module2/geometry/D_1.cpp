//D1. Возможно глупая ссора
//        Ограничение времени	1 секунда
//        Ограничение памяти	64Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//        Арсений опять уснул. И снова на планете Ка-Пэкс без него никак —
//        два фермера поссорились из-за того, что их территории могут перекрываться.
//        Естественно, как самого рассудительного, Арсения позвали урегулировать конфликт.
//        Для начала он решил узнать, насколько серьезен конфликт. Для этого он решил узнать,
//        пересекаются ли фермы вообще. Помогите Арсению это понять, если известно,
//        что каждая ферма представляет собой выпуклый многоугольник.
//
//Формат ввода
//  Первая строка содержит число N точек первого многоугольника.
//  Затем идут N строк с координатами точек первого многоугольника по
//  часовой стрелке (координаты – действительные числа).
//
//  Второй прямоугольник задается аналогично. N, M ≤ 80000.
//
//Формат вывода
//  Выведите “YES”, если фермы пересекаются, и “NO”, если нет (без кавычек).
//
// Реализуйте алгоритм за O(N+M). (СУММА МИНКОВСКОГО)

#include <iostream>
#include <vector>
#include "geometry2D.cpp"

template<typename T>
Polygon<T> readPolygon(std::istream & in) {
    size_t N;
    in >> N;

    std::vector<Point<T>> vertices;
    vertices.reserve(N);

    for (size_t i = 0; i < N; ++i) {
        Point<T> p{};
        std::cin >> p;
        vertices.emplace_back(p);
    }

    return Polygon<T>(vertices);
}

int
main() {
    using T = double;

    auto K = readPolygon<T>(std::cin);
    auto L_ = readPolygon<T>(std::cin);
    // make reflected polygon L_
    L_.reflex({0, 0});

    // make Minkovskiy sum
    auto sum = K + L_;

    // check if "sum contains Point(0, 0)". It is equivalents to "K intersects L".
    std::cout << (sum.containsPoint({0, 0}) ? "YES" : "NO") << std::endl;
}