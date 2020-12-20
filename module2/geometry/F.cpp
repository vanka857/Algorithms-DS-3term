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
        out << i << std::endl;
    }
}

int main() {
    using T = long double;

    std::vector<Point3DChan<T>> points_Chan;

    size_t id = 0;

    T x, y;
    while (std::cin && std::cin >> x && std::cin >> y) {

        Point3DChan<T> p(x, y, x*x + y*y, id);

        points_Chan.emplace_back(p);
        id++;
    }

    // add to infinite points
    Point3DChan<T> p1(INF_F, -INF_F+100, 2 * INF_F * INF_F, -1);
    Point3DChan<T> p2(-INF_F, INF_F-100, 2 * INF_F * INF_F, -2);

    points_Chan.push_back(p1);
    points_Chan.push_back(p2);

    // build lower convex hull using Chan algo
    ConvexHull3D<T> cch3(points_Chan);
    auto lower_hull_3D = cch3.partialHull(ConvexHull3D<T>::LOWER_HULL, ConvexHull3D<T>::xLess_zGreater);

    // если нет ограниченных многоугольнков
    if (lower_hull_3D.empty()) {
        std::cout << 0;
        return 0;
    }

    std::set<int> fake_points;

    for (const auto & plane : lower_hull_3D) {
        if ((plane.vertices[0].id < 0) ||
            (plane.vertices[1].id < 0) ||
            (plane.vertices[2].id < 0)) {

            fake_points.insert(plane.vertices[0].id);
            fake_points.insert(plane.vertices[1].id);
            fake_points.insert(plane.vertices[2].id);
        }
    }

    size_t edges_count = 0;

    auto is_fake = [&fake_points](int64_t index){
        return (fake_points.count(index) == 1);
    };

    for (const auto &plane : lower_hull_3D) {
        if (!is_fake(plane.vertices[0].id))
            ++edges_count;

        if (!is_fake(plane.vertices[1].id))
            ++edges_count;

        if (!is_fake(plane.vertices[2].id))
            ++edges_count;
    }

    size_t points_count = points_Chan.size() - fake_points.size();
    if (points_count == 0) {
        std::cout << 0;
    } else {
        std::cout << std::setprecision(7) << static_cast<long double>(edges_count) / points_count;
    }

}
