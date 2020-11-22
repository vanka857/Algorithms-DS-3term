//С1. Кубик мечты
//Ограничение времени	0.2 секунды
//        Ограничение памяти	1Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//        Арсений решил поиграть в настольную игру со своим другом Ильей. Так как играть обычными кубиками им
//        уже стало неинтересно, Илья попросил по его чертежам сконструировать новую игральную кость.
//        Так как он ленивый, то просто накидал точек в пространстве и сказал, что все они должны
//        лежать в кубике его мечты. У Арсения есть 3D-принтер, вот только материалы для печати достаточно дорогие,
//        поэтому он хочет выполнить требования своего друга, потратив как можно меньше пластика.
//
//Формат ввода
//  Первая строка содержит число M — количество тестов. В последующих строках описаны сами тесты.
//  Каждый тест начинается со строки, содержащей N (N ≤ 1000) — число точек. Далее, в N строках даны по три числа —
//  координаты точек. Все координаты целые, не превосходят по модулю 500.
//
//Формат вывода
//  Для каждого теста выведите следующее. В первую строку выведите количество граней m.
//  Далее в последующие m строк выведите описание граней: количество точек грани (=3)
//  и номера точек в исходном множестве.
//  Точки нумеруются в том же порядке, в котором они даны во входном файле.
//  Точки в пределах грани должны быть отсортированы в порядке против часовой стрелки
//  относительно внешней нормали к грани. Первая точка – точка с минимальным номером.
//  Порядок граней лексикографический.

#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include "geometry3D.cpp"

template<typename T>
void printVector(const std::vector<T> & data, std::ostream & out) {
    for (const auto & i : data) {
        out << i.toString() << std::endl;
    }
}

int main() {
    using T = double;

    size_t M;
    std::cin >> M;

    for (size_t i = 0; i < M; ++i) {
        size_t N;
        std::cin >> N;

        std::vector<Vertex<T> > points;
        points.reserve(N);

        for (size_t j = 0; j < N; ++j) {
            auto p = readPoint3D<T>(std::cin, j);
            p.rotate(0.05);
            points.emplace_back(p.x, p.y, p.z, p.id);
        }

        ConvexHull3DClass cch(points);
        auto ch = cch.hull();

        std::sort(ch.begin(), ch.end(), [](Face<T> & lhs, Face<T> & rhs){
            if (lhs.vertices[0].id == rhs.vertices[0].id) {
                if (lhs.vertices[1].id == rhs.vertices[1].id) {
                    return lhs.vertices[2].id < rhs.vertices[2].id;
                } else {
                    return lhs.vertices[1].id < rhs.vertices[1].id;
                }
            }
            else {
                return lhs.vertices[0].id < rhs.vertices[0].id;
            }
        });

        std::cout << ch.size() << std::endl;
        printVector(ch, std::cout);
    }

}