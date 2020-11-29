//F. Диаграмма Вороного
//Ограничение времени	1 секунда
//        Ограничение памяти	128Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//
//        Даны точки, никакие 3 из которых не лежат на одной прямой.
//        Никакие 4 точки не лежат на одной окружности.
//        Кроме того, все точки имеют различные x-координаты.
//
//        Определите среднее число сторон в многоугольниках диаграммы Вороного этого множества точек.
//        Считаются только конечные многоугольники.
//        Если все многоугольники неограниченны, ответ полагается равным 0.
//
//        Число точек n ≤ 100000. Алгоритм должен иметь асимптотику O(n log n).
//
//Формат ввода
// В каждой строке через пробел записаны действительные координаты точек xi yi.
//
//Формат вывода
// Число - среднее число сторон в ограниченных многоугольниках диаграммы Вороного с точностью 10^-6.
// Если таких многоугольников нет - ответ 0.

#include <set>
#include <iomanip>
#include "geometry3D.cpp"

template<typename T>
void printVector(const std::vector<T> & data, std::ostream & out) {
    for (const auto & i : data) {
        out << i.toString() << std::endl;
    }
}

int main() {
    using T = long double;

    std::vector<Vertex<T>> vertices3D;

    size_t id = 0;

    T x, y, z;
    while (std::cin && std::cin >> x && std::cin >> y) {

        Vertex<T> p(x, y, x*x + y*y, id);

        vertices3D.emplace_back(p);
        id++;
    }

    // add to infinite points
    Vertex<T> p1(INF_F, -INF_F+100, 2 * INF_F * INF_F, -1);
    Vertex<T> p2(-INF_F, INF_F-100, 2 * INF_F * INF_F, -2);

    vertices3D.push_back(p1);
    vertices3D.push_back(p2);

    // build lower convex hull using Chan algo
    ConvexHull3DClass<T> cch3(vertices3D);
    auto lowerHull3D = cch3.lowerHull(ConvexHull3DClass<T>::xLess_zGreater);

    // если нет ограниченных многоугольнков
    if (lowerHull3D.empty()) {
        std::cout << 0;
        return 0;
    }

    std::set<int> fakePoints;

    for (const auto & plane : lowerHull3D) {
        if ((plane.vertices[0].id < 0) ||
            (plane.vertices[1].id < 0) ||
            (plane.vertices[2].id < 0)) {

            fakePoints.insert(plane.vertices[0].id);
            fakePoints.insert(plane.vertices[1].id);
            fakePoints.insert(plane.vertices[2].id);
        }
    }

    size_t edgesCount = 0;

    auto isFake = [&fakePoints](int64_t index){
        return (fakePoints.count(index) == 1);
    };

    for (const auto &plane : lowerHull3D) {
        if (!isFake(plane.vertices[0].id))
            ++edgesCount;

        if (!isFake(plane.vertices[1].id))
            ++edgesCount;

        if (!isFake(plane.vertices[2].id))
            ++edgesCount;
    }

    size_t pointsCount = vertices3D.size() - fakePoints.size();
    if (pointsCount == 0) {
        std::cout << 0;
    } else {
        std::cout << std::setprecision(7) << static_cast<long double>(edgesCount) / pointsCount;
    }

}
