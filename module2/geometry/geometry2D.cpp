#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

enum class orientation {
    L,
    R,
    EQ
};

template<typename T>
bool is_equal(const T & x, const T & y) {
    return x == y;
}
bool is_equal(long double x, long double y) {
    return std::fabs(x - y) <= std::numeric_limits<long double>::epsilon();
}

template <typename T>
bool is_less_equal(const T & x, const T & y) {
    return is_equal(x, y) || x < y;
}
template<typename T>
class Point {
    // TODO make x, y private, make getter, setter, etc...
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

    template <typename T1> friend std::istream& operator>> (std::istream& in, Point<T>& point);
    template <typename T1> friend std::ostream& operator<< (std::ostream& out, const Point<T>& point);
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
struct xLess_yLess{
    bool operator()(Point<T> a, Point<T> b) const {
        if(a.x < b.x) {
            return true;
        } else if (a.x == b.x) {
            return a.y < b.y;
        }
        return false;
    }
} ;

template<typename T>
Point<T> & Point<T>::operator+=(const Point & rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
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
std::istream & operator>>(std::istream & in, Point<T>& point) {
    in >> point.x >> point.y;
    return in;
}

template<typename T>
std::ostream & operator<<(std::ostream & out, const Point<T> & point) {
    out << point.x << point.y;
    return out;
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
class Vector {
    // TODO make begin, end private ?
public:
    Point<T> begin;
    Point<T> end;

public:
    explicit Vector(const Point<T> & point): begin({0, 0}), end(point) {}
    Vector(const Point<T> & beg, const Point<T> & end): begin(beg), end(end) {}

    Vector operator - () const;

    T x() const;
    T y() const;
};

template<typename T>
Vector<T> Vector<T>::operator-() const {
    return {this->end, this->begin};
}
template<typename T>
T Vector<T>::x() const {
    return end.x - begin.x;
}
template<typename T>
T Vector<T>::y() const {
    return end.y - begin.y;
}

template<typename T>
Vector<T> operator + (const Vector<T> & lhs, const Vector<T> & rhs) {
    return {lhs.begin, {lhs.x() + rhs.x(), lhs.y() + rhs.y()}};
}
template<typename T>
Vector<T> operator - (const Vector<T> & lhs, const Vector<T> & rhs) {
    return lhs + -rhs;
}
template<typename T, typename D>
Vector<T> operator * (const Vector<T> & lhs, D factor) {
    return {{lhs.begin.x * factor, lhs.begin.y * factor}, {lhs.end.x * factor, lhs.end.y * factor}};
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

    if (is_equal(crPr1, static_cast<T>(0)) && (belongsToParallel(a1, b1, b2) || belongsToParallel(a2, b1, b2))) {
        return true;
    }

    if (is_equal(crPr2, static_cast<T>(0)) && (belongsToParallel(b1, a1, a2) || belongsToParallel(b2, a1, a2))) {
        return true;
    }

    return false;
}

template<typename T>
inline bool belongsToParallel(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // states that p1 belongs to (p2; p3) (line)
    // return true if p1 belongs to [p2; p3] (line segment).

    return is_equal(crossProduct<T>({p1, p2}, {p1, p3}), static_cast<T>(0)) && is_less_equal(dotProduct<T>({p1, p2}, {p1, p3}), static_cast<T>(0));
}

template<typename T>
inline T crossProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // (1, 2) - first vector, (1, 3) - second
    Point a = p2 - p1;
    Point b = p3 - p1;
    return a.x * b.y - a.y * b.x;
}

template<typename T>
inline T crossProduct(const Vector<T> & v1, const Vector<T> & v2) {
    return v1.x() * v2.y() - v1.y() * v2.x();
}

template<typename T>
inline T dotProduct(const Point<T> & p1, const Point<T> & p2, const Point<T> & p3) {
    // (1, 2) - first vector, (1, 3) - second
    Point a = p2 - p1;
    Point b = p3 - p1;
    return a.x * b.x + a.y * b.y;
}

template<typename T>
inline T dotProduct(const Vector<T> & v1, const Vector<T> & v2) {
    return v1.x() * v2.x() + v1.y() * v2.y();
}

template<typename T>
class Polygon;

template<typename Container, typename T>
class ConvexHull {
public:
    typename Container::iterator begin, end;

    explicit ConvexHull(typename Container::iterator begin, typename Container::iterator end) :
            begin(begin), end(end) {
        // TODO check iterator tag >= bidirectional

        std::sort(begin, end, xLess_yGreater<T>());
    }
    Polygon<T> createConvexHull();

private:
    // TODO return Polygon instead of vector<Point>
    template <typename Iterator>
    std::vector<Point<T>> createConvexHullOfSorted(Iterator begin_, Iterator end_);
};

template<typename Container, typename T>
Polygon<T> ConvexHull<Container, T>::createConvexHull() {
    std::vector<Point<T>> top = createConvexHullOfSorted<typename Container::reverse_iterator>(
            std::make_reverse_iterator(end), std::make_reverse_iterator(begin));
    top.pop_back();

    std::vector<Point<T>> bottom = createConvexHullOfSorted<typename Container::iterator>(begin, end);
    bottom.pop_back();

    std::move(bottom.begin(), bottom.end(), std::back_inserter(top));

    if(std::is_same_v<Container, std::vector<Point<T>>>) {
        return Polygon<T>(top);
    }
    else {
        // construct vector and return;
    }
}

template<typename Container, typename T>
template<typename Iterator>
std::vector<Point<T>> ConvexHull<Container, T>::createConvexHullOfSorted(Iterator begin_, Iterator end_) {
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

template<typename T>
class Shape {
public:
    virtual ~Shape() = default;

    virtual bool containsPoint(const Point<T> & point) const = 0;

    virtual void reflex(const Point<T> & center) = 0;
};


template <typename T>
class Polygon : public Shape<T> {
private:
    std::vector<Point<T>> vertices;
    Polygon() = default;
    void setVertices(const std::vector<Point<T>> & vertices);

    bool reflected = false;

public:
    explicit Polygon(const std::vector<Point<T>> & vertices);
    explicit Polygon(const std::vector<Vector<T>> & edges);
    template <typename  ...Points>
    explicit Polygon(Points... points);

    [[nodiscard]] size_t verticesCount() const;
    std::vector<Point<T>> getVertices() const;
    std::vector<Vector<T>> getEdges() const;

    void move(const Vector<T> & shift);

    void update_min();

    [[nodiscard]] bool is_reflected() const { return reflected; }

    void print() const;
    [[nodiscard]] std::string toString() const;

    bool containsPoint(const Point<T> & point) const override;

    void reflex(const Point<T> & center) override;
};

template <typename  T>
Polygon<T>::Polygon(const std::vector<Point<T>> & vertices) {
    this->vertices = vertices;
    update_min();
}
template <typename  T>
template <typename  ...Points>
Polygon<T>::Polygon(Points... points) : Polygon(std::vector<Point<T>>({static_cast<Point<double>>(points)...})) {}

template<typename T>
Polygon<T>::Polygon(const std::vector<Vector<T>> & edges) {
    vertices.push_back({0, 0});

    for (size_t i = 0; i < edges.size() - 1; ++i) {
        auto temp = Vector(vertices[i]) + edges[i];
        vertices.emplace_back(temp.x(), temp.y());
    }

    update_min();
}

template<typename T>
void Polygon<T>::move(const Vector<T> & shift) {
    for(auto & vertex : vertices) {
        vertex += {shift.x(), shift.y()};
    }
}

template<typename T>
void Polygon<T>::update_min() {

    Point<T> minPoint(std::numeric_limits<T>::max(), std::numeric_limits<T>::min());
    typename std::vector<Point<T>>::iterator min_pos;

    for (size_t i = 0; i < verticesCount(); ++i) {
        auto & point = vertices[i];
        auto cmp = xLess_yLess<T>();

        if (cmp(point, minPoint)) {

            minPoint = point;
            min_pos = vertices.begin() + i;
        }
    }

    std::vector<Point<T>> vertices1(min_pos, vertices.end());
    std::move(vertices.begin(), min_pos, std::back_inserter(vertices1));

    vertices = vertices1;
}

template <typename  T>
inline size_t Polygon<T>::verticesCount() const {
    return vertices.size();
}
template <typename  T>
std::vector<Point<T>> Polygon<T>::getVertices() const {
    return vertices;
}
template<typename T>
std::vector<Vector<T>> Polygon<T>::getEdges() const {
    std::vector<Vector<T>> result;
    result.reserve(verticesCount());

    for (size_t i = 1; i < verticesCount(); ++i) {
        result.emplace_back(vertices[i - 1], vertices[i]);
    }

    result.emplace_back(*(vertices.end() - 1), vertices[0]);

    return result;
}

template <typename  T>
bool Polygon<T>::containsPoint(const Point<T> & point) const{
    const auto & edges = getEdges();

    for (const auto & edge : edges) {
        if (crossProduct(point, edge.begin, edge.end) > 0) {
            return false;
        }
    }

    return true;
}

template<typename T>
Polygon<T> operator + (const Polygon<T> lhs, const Polygon<T> rhs) {
    // TODO check if vertices (and edges) in lhs, rhs sorted
    auto e_lhs = lhs.getEdges();
    auto e_rhs = rhs.getEdges();

    std::vector<Vector<T>> sorted_edges;

    auto less_ = [](Vector<T> lhs, Vector<T> rhs){
        return crossProduct(lhs, rhs) < 0;
    };

    std::merge(e_lhs.begin(), e_lhs.end(), e_rhs.begin(), e_rhs.end(), std::back_inserter(sorted_edges), less_);

    Polygon raw(sorted_edges);
    raw.move(Vector(e_lhs[0].begin) + Vector(e_rhs[0].begin));

    return raw;
}

template <typename T>
void reflexPoints(std::vector<Point<T>> & vertices, const Point<T> & center) {
    for (auto & p : vertices) {
        auto temp = Vector<T>(center) * 2 - Vector<T>(p);
        p.x = temp.x();
        p.y = temp.y();
    }
}

template <typename T>
void Polygon<T>::reflex(const Point<T> & center) {
    reflexPoints(vertices, center);
    reflected = !reflected;
    update_min();
}

template <typename T>
void Polygon<T>::print() const {
    // TODO print in parameter istream
    std::cout << this->toString() << std::endl;
}

template <typename T>
std::string Polygon<T>::toString() const {
    std::string result;
    for (auto const & p : this->vertices) {
        result += "[" + p.toString() + "] ";
    }
    return result;
}

template <typename T>
void Polygon<T>::setVertices(const std::vector<Point<T>> & vertices_) {
    Polygon::vertices = vertices_;
}
