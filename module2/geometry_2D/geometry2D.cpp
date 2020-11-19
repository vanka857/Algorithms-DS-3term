#include <iostream>
#include <algorithm>
#include <vector>

enum class orientation {
    L,
    R,
    EQ
};

template<typename T>
class Point {
public:
    T x;
    T y;

public:
    Point() = default;
    Point(T x, T y) : x(x), y(y) {}

    Point & operator+=(const Point & rhs);
    Point & operator-=(const Point & rhs);
    Point operator-() const;

    [[nodiscard]] double len() const;
};

template<typename T>
struct xLess_yGreater{
    bool operator()(Point<T> a, Point<T> b) const {
        if(a.x < b.x) {
            return true;
        } else if (a.x == b.x) {
            return a.y > b.y;
        }
        return false;
    }
} ;

template<typename T>
class Vector {
private:
    Point<T> begin;
    Point<T> end;

public:
    Vector(const Point<T> & beg, const Point<T> & end): begin(beg), end(end) {}

    T x() const;
    T y() const;
};

template<typename T>
T Vector<T>::x() const {
    return end.x - begin.x;
}

template<typename T>
T Vector<T>::y() const {
    return end.y - begin.y;
}

template<typename T>
Point<T> & Point<T>::operator+=(const Point & rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return this;
}

template<typename T>
Point<T> & Point<T>::operator-=(const Point & rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

template<typename T>
Point<T> Point<T>::operator-() const{
    return {-this->x, -this->y};
}

template<typename T>
double Point<T>::len() const {
    return sqrt(x * x + y * y);
}

template<typename T>
Point<T> operator+(Point<T> lhs, const Point<T> & rhs) {
    lhs += rhs;
    return lhs;
}

template<typename T>
Point<T> operator-(Point<T> lhs, const Point<T> & rhs) {
    lhs -= rhs;
    return lhs;
}

template<typename T>
bool intersects(const Point<T> & a1, const Point<T> & a2, const Point<T> & b1, const Point<T> & b2);

template<typename T>
bool belongsToParallel(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3);

template<typename T>
T crossProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3);
template<typename T>
T crossProduct(const Vector<T> & v1, const Vector<T> & v2);

template<typename T>
T dotProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3);
template<typename T>
T dotProduct(const Vector<T> & v1, const Vector<T> & v2);

template<typename T>
bool intersects(const Point<T> & a1, const Point<T> & a2, const Point<T> & b1, const Point<T> & b2) {
    // return true, if [a1, a2] intersects [b1, b2]
    Vector b(b1, b2);
    Vector a(a1, a2);

    T crPr1 = crossProduct(b, {b1, a2}) * crossProduct(b, {b1, a1});
    T crPr2 = crossProduct(a, {a1, b2}) * crossProduct(a, {a1, b1});

    if (crPr1 < 0 && crPr2 < 0) {
        return true;
    }

    if (crPr1 == 0 && (belongsToParallel(a1, b1, b2) || belongsToParallel(a2, b1, b2))) {
        return true;
    }

    if (crPr2 == 0 && (belongsToParallel(b1, a1, a2) || belongsToParallel(b2, a1, a2))) {
        return true;
    }

    return false;
}

template<typename T>
bool belongsToParallel(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // states that p1 belongs to (p2; p3) (line)
    // return true if p1 belongs to [p2; p3] (line segment).

    return crossProduct<T>({p1, p2}, {p1, p3}) == 0 && dotProduct<T>({p1, p2}, {p1, p3}) <= 0;
}

template<typename T>
T crossProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // (1, 2) - first vector, (1, 3) - second
    Point a = p2 - p1;
    Point b = p3 - p1;
    return a.x * b.y - a.y * b.x;
}

template<typename T>
T crossProduct(const Vector<T> & v1, const Vector<T> & v2) {
    return v1.x() * v2.y() - v1.y() * v2.x();
}

template<typename T>
T dotProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // (1, 2) - first vector, (1, 3) - second
    Point a = p2 - p1;
    Point b = p3 - p1;
    return a.x * b.x + a.y * b.y;
}

template<typename T>
T dotProduct(const Vector<T> & v1, const Vector<T> & v2) {
    return v1.x() * v2.x() + v1.y() * v2.y();
}

template<typename T>
Point<T> readPoint(std::istream & in) {
    T x, y;
    in >> x >> y;
    return {x, y};
}


template<typename Container, typename T>
class ConvexHullClass {
public:
    typename Container::iterator begin, end;

    explicit ConvexHullClass(typename Container::iterator begin, typename Container::iterator end) :
        begin(begin), end(end) {
        // TODO check iterator tag >= bidirectional

        std::sort(begin, end, xLess_yGreater<T>());
    }
    std::vector<Point<T>> createConvexHull();

private:
    template <typename Iterator>
    std::vector<Point<T>> createConvexHullOfSorted(Iterator begin_, Iterator end_);
};

template<typename Container, typename T>
std::vector<Point<T>> ConvexHullClass<Container, T>::createConvexHull() {
    std::vector<Point<T>> top = createConvexHullOfSorted<typename Container::reverse_iterator>(
            std::make_reverse_iterator(end), std::make_reverse_iterator(begin));
    top.pop_back();

    std::vector<Point<T>> bottom = createConvexHullOfSorted<typename Container::iterator>(begin, end);
    bottom.pop_back();

    std::move(bottom.begin(), bottom.end(), std::back_inserter(top));

    return top;
}

template<typename Container, typename T>
template<typename Iterator>
std::vector<Point<T>> ConvexHullClass<Container, T>::createConvexHullOfSorted(Iterator begin_, Iterator end_) {
    std::vector<Point<T>> result;
    result.reserve(end_ - begin_);

    // paste first two points to result vector
    std::move(begin_, begin_ + 2, std::back_inserter(result));

    for(auto p3_ptr = begin_ + 2; p3_ptr < end_; ) {
        Point<T> p3 = *p3_ptr;

        if (result.size() >= 2) {
            const Point<T> & p2 = result.rbegin()[0];
            const Point<T> & p1 = result.rbegin()[1];

            if (crossProduct(p1, p2, p3) <= 0) {
                // pop result, continue
                result.pop_back();
                continue;
            }
        }
        result.push_back(p3);
        ++p3_ptr;
    }

    return result;
}
