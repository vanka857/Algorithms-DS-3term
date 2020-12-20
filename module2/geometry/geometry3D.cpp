#include "geometry2D.cpp"
#include <algorithm>
#include <vector>
#include <cmath>
#include <functional>
#include <string>

const static double INF_LD = 1e40; // std::numeric_limits<double>::max();
const static double INF_F = 1e10; // std::numeric_limits<float>::max();

template<typename T>
class Point3D {
public:
    T x, y, z;

    int id = -1;

    Point3D() = default;
    Point3D(const T & x, const T & y, const T & z, int id) : x(x), y(y), z(z), id(id) {}

    void rotate(long double angle) {
        T new_z = z * cosl(angle) + y * sinl(angle);
        T new_y = -z * sinl(angle) + y * cosl(angle);
        z = new_z;
        y = new_y;

        T new_x = x * cosl(angle) + z * sinl(angle);
        new_z = -x * sinl(angle) + z * cosl(angle);
        x = new_x;
        z = new_z;

        new_x = x * cosl(angle) + y * sinl(angle);
        new_y = -x * sinl(angle) + y * cosl(angle);
        x = new_x;
        y = new_y;
    }

    template <typename T1> friend std::istream& operator>> (std::istream& in, Point3D<T>& point);
    template <typename T1> friend std::ostream& operator>> (std::ostream& out, Point3D<T>& point);
};

template<typename T>
std::istream & operator>>(std::istream & in, Point3D<T>& point) {
    in >> point.x >> point.y >> point.z;
    return in;
}

template<typename T>
std::ostream & operator>>(std::ostream & out, Point3D<T>& point) {
    out << point.x << point.y << point.z;
    return out;
}

// Этот класс содержит методы, нужные для алгоритма Чана
template<typename T>
class Point3DChan : public Point3D<T> {
public:
    Point3DChan<T> * prev = nullptr;
    Point3DChan<T> * next = nullptr;

    Point3DChan(const T & x, const T & y, const T & z, int id) : Point3D<T>(x, y, z, id) {}

    bool updateLinks() {
        if (prev->next == this) {
            prev->next = next;
            next->prev = prev;

            return true;
        }
        else {
            prev->next = this;
            next->prev = this;

            return false;
        }
    }
};

template<typename T>
struct Face {
    std::vector<Point3DChan<T>> vertices;

    Face() = default;
    explicit Face(const std::vector<Point3DChan<T> > & points) : vertices(points) {}
    Face(Point3DChan<T> v1, Point3DChan<T> v2, Point3DChan<T> v3) {
        vertices.emplace_back(v1);
        vertices.emplace_back(v2);
        vertices.emplace_back(v3);
    }

    void getLessOrder();

    template<typename T1> friend std::ostream& operator<< (std::ostream& out, const Face<T>& face);
};

template<typename T>
std::ostream& operator<< (std::ostream& out, const Face<T>& face) {
    out << face.vertices.size();
    for (const auto & vertex : face.vertices) {
        out << " " << vertex.id;
    }
    return out;
}

template<typename T>
void Face<T>::getLessOrder() {
    // вращение грани, чтобы первой точкой была наименьшая по номеру (сохраняется относительный порядок точек в грани)
    if (vertices[1].id < vertices[0].id  && vertices[1].id < vertices[2].id) {
        std::swap(vertices[0], vertices[1]);
        std::swap(vertices[1], vertices[2]);
    } else if (vertices[2].id < vertices[0].id && vertices[2].id < vertices[1].id) {
        std::swap(vertices[1], vertices[2]);
        std::swap(vertices[0], vertices[1]);
    }
}

template<typename T>
class ConvexHull3D {
private:
    std::vector<Point3DChan<T> > points;

public:
    enum {
        LOWER_HULL = 0,
        UPPER_HULL = 1,
    };

    static bool xLess_zGreater(Point3D<T> lhs, Point3D<T> rhs) {
        return lhs.x < rhs.x || (is_equal(lhs.x, rhs.x) && lhs.z > rhs.z);
    }

    static bool zLess_xGreater(Point3D<T> lhs, Point3D<T> rhs) {
        return lhs.z < rhs.z || (is_equal(lhs.x, rhs.x) && lhs.x > rhs.x);
    }

    explicit ConvexHull3D(const std::vector<Point3DChan<T> > & points) : points(points) {}
    std::vector<Face<T>> hull() const; // return ConvexHull as faces

    // return partial (lower or upper) ConvexHull as faces
    std::vector<Face<T>> partialHull(bool hull = LOWER_HULL, std::function<bool (Point3DChan<T>, Point3DChan<T>)> less = xLess_zGreater) const;

private:

    enum EventType {
        LEFT,
        RIGHT,
        LEFT_BRIDGE_PREV,
        LEFT_BRIDGE_NEXT,
        RIGHT_BRIDGE_PREV,
        RIGHT_BRIDGE_NEXT,
    };

    void sortPoints(std::vector<Point3DChan<T>> & source, std::function<bool (Point3DChan<T>, Point3DChan<T>)> less) const;
    static std::vector<Point3DChan<T> > reflected(typename std::vector<Point3DChan<T>>::iterator begin, typename std::vector<Point3DChan<T>>::iterator end) {
        std::vector<Point3DChan<T> > result;

        for (auto it = begin; it < end; ++it) {
            auto pt = *it;
            Point3DChan<T> v(pt.x, pt.y, - pt.z, pt.id);
            v.prev = nullptr;
            v.next = nullptr;
            result.push_back(v);
        }

        return result;
    }

    static std::vector<Point3DChan<T> *> hull_raw(std::vector<Point3DChan<T> *> & pts, size_t from, size_t to) ; // return ConvexHull of part [begin, end) as ordered vector of ptr to points
    static std::vector<Point3DChan<T> *> merge(const std::vector<Point3DChan<T> *> & left, const std::vector<Point3DChan<T> *> & right, Point3DChan<T> * left_br, Point3DChan<T> * right_br); // merge two ConvexHulls

    static void buildBridges(std::vector<Point3DChan<T> *> & data, Point3DChan<T>* & left_br, Point3DChan<T>* & right_br, T x);
    static std::pair<Point3DChan<T> *, Point3DChan<T> *> findBridge(Point3DChan<T> * left_br, Point3DChan<T> * right_br);

    static T turnTime(Point3DChan<T> * a, Point3DChan<T> * b, Point3DChan<T> * c);
    static bool isRightTurn(Point3DChan<T> * a, Point3DChan<T> * b, Point3DChan<T> * c);

    static void pushFaces(std::vector<Face<T>> & faces, const std::vector<Point3DChan<T> *> & points, bool reflected = false);
    static std::vector<Point3DChan<T> *> createReferences(std::vector<Point3DChan<T>> & points);
};

template <typename T>
void ConvexHull3D<T>::pushFaces(std::vector<Face<T>> & faces, const std::vector<Point3DChan<T> *> & points, bool reflected) {
    // Эта функция возвращает грани в виде трех точек,
    // отсортированных по часовой стрелке относительно внешней нормали.

    for (auto * point : points) {
        // Переупорядочим точки в грани если порядок был нарушен
        // (point->updateLinks() == true)
        // Или если мы вставляем отраженные грани
        Face<T> face;

        if (reflected ^ point->updateLinks()) {
            face = Face<T>(*point, *(point->prev), *(point->next));
        } else {
            face = Face<T>(*(point->prev), *point, *(point->next));
        }

        // поставим вершину с минимальным номером на первое место в грани
        face.getLessOrder();
        faces.emplace_back(face);
    }
}

template <typename T>
std::vector<Point3DChan<T> *> ConvexHull3D<T>::createReferences(std::vector<Point3DChan<T>> & points) {
    std::vector<Point3DChan<T> *> result;

    for (auto & p : points) {
        result.push_back(&p);
    }

    return result;
}

template<typename T>
std::vector<Face<T>> ConvexHull3D<T>::partialHull(bool hull, std::function<bool (Point3DChan<T>, Point3DChan<T>)> less) const {
    std::vector<Face<T>> result;

    auto sorted = points;
    sortPoints(sorted, less);

    if (hull == UPPER_HULL) {
        sorted = reflected(sorted.begin(), sorted.end());
    }

    auto sorted_references = createReferences(sorted);
    auto ch = hull_raw(sorted_references, 0, sorted_references.size());

    pushFaces(result, ch, (hull == UPPER_HULL));

    return result;
}

template<typename T>
std::vector<Face<T>> ConvexHull3D<T>::hull() const {
    auto lch = partialHull(LOWER_HULL);
    auto uch = partialHull(UPPER_HULL);

    std::move(uch.begin(), uch.end(), std::back_inserter(lch));

    return lch;
}


template<typename T>
void ConvexHull3D<T>::sortPoints(std::vector<Point3DChan<T>> & source, std::function<bool (Point3DChan<T>, Point3DChan<T>)> less) const {
    std::sort(source.begin(), source.end(), less);
}

template<typename T>
std::vector<Point3DChan<T> *>
ConvexHull3D<T>::hull_raw(std::vector<Point3DChan<T> *> & pts, size_t from, size_t to) {

    if (from + 1 == to) { // n == 1
        std::vector<Point3DChan<T> *> result;
        result.push_back(pts[from]);
        return result;
    }

    size_t mid = from + static_cast<int>((to - from) / 2);

    auto r1 = hull_raw(pts, from, mid);
    auto r2 = hull_raw(pts, mid, to);

    return  merge(r1, r2, pts[mid - 1], pts[mid]);
}

template <typename T>
T ConvexHull3D<T>::turnTime(Point3DChan<T> * a, Point3DChan<T> * b, Point3DChan<T> * c) {
    if (a == nullptr || b == nullptr || c == nullptr) {
        return INF_LD;
    }
    Vector<T> ab_y({a->x, a->y}, {b->x, b->y});
    Vector<T> bc_y({b->x, b->y}, {c->x, c->y});

    Vector<T> ab_z({a->x, a->z}, {b->x, b->z});
    Vector<T> bc_z({b->x, b->z}, {c->x, c->z});

    T temp = crossProduct(ab_y, bc_y);
    return is_equal(temp,  static_cast<T>(0)) ? INF_LD : crossProduct(ab_z, bc_z) / temp;
}

template <typename T>
bool ConvexHull3D<T>::isRightTurn(Point3DChan<T> * a, Point3DChan<T> * b, Point3DChan<T> * c) {
    if (a == nullptr || b == nullptr || c == nullptr) {
        return false;
    }

    Vector<T> ab_y({a->x, a->y}, {b->x, b->y});
    Vector<T> bc_y({b->x, b->y}, {c->x, c->y});

    return is_less_equal(crossProduct(ab_y, bc_y), static_cast<T>(0));
}

template<typename T>
std::pair<Point3DChan<T> *, Point3DChan<T> *>
ConvexHull3D<T>::findBridge(Point3DChan<T> * left_br,
                            Point3DChan<T> * right_br) {

    while (true) {
        if (isRightTurn(left_br, right_br, right_br->next)) {
            right_br = right_br->next;
        } else if (isRightTurn(left_br->prev, left_br, right_br)) {
            left_br = left_br->prev;
        } else {
            break;
        }
    }

    return {left_br, right_br};
}

template<typename T>
std::vector<Point3DChan<T> *>
ConvexHull3D<T>::merge(const std::vector<Point3DChan<T> *> & left,
                       const std::vector<Point3DChan<T> *> & right,
                       Point3DChan<T> * left_br_, Point3DChan<T> * right_br_) {

    std::vector<Point3DChan<T> *> result;

    auto x = left_br_->x;
    auto bridge = findBridge(left_br_, right_br_);

    auto [left_br, right_br] = bridge;

    T time = -INF_LD;

    auto l_it = left.begin();
    auto r_it = right.begin();


    while (true) {
        T min_time = INF_LD;

        T temp;
        EventType event;

        Point3DChan<T> * l;
        Point3DChan<T> * r;

        if (l_it != left.end() && (l = *l_it, temp = turnTime(l->prev, l, l->next), time <= temp && temp < min_time)) {
            min_time = temp;
            event = LEFT;
        }
        if (r_it != right.end() && (r = *r_it, temp = turnTime(r->prev, r, r->next), time <= temp && temp < min_time)) {
            min_time = temp;
            event = RIGHT;
        }
        if (temp = turnTime(left_br->prev, left_br, right_br), time < temp && temp < min_time) {
            min_time = temp;
            event = LEFT_BRIDGE_PREV;
        }
        if (temp = turnTime(left_br, left_br->next, right_br), time < temp && temp < min_time) {
            min_time = temp;
            event = LEFT_BRIDGE_NEXT;
        }
        if (temp = turnTime(left_br, right_br->prev, right_br), time < temp && temp < min_time) {
            min_time = temp;
            event = RIGHT_BRIDGE_PREV;
        }
        if (temp = turnTime(left_br, right_br, right_br->next), time < temp && temp < min_time) {
            min_time = temp;
            event = RIGHT_BRIDGE_NEXT;
        }

        if (is_equal(min_time, INF_LD)) break;

        time = min_time;

        switch(event) {
            case LEFT:
                if (l->x < left_br->x)
                    result.push_back(l);
                l->updateLinks();
                if (l_it != left.end())
                    l_it++;
                break;
            case RIGHT:
                if (r->x > right_br->x)
                    result.push_back(r);
                r->updateLinks();
                if (r_it != right.end())
                    r_it++;
                break;
            case LEFT_BRIDGE_PREV:
                result.push_back(left_br);
                left_br = left_br->prev;
                break;
            case LEFT_BRIDGE_NEXT:
                left_br = left_br->next;
                result.push_back(left_br);
                break;
            case RIGHT_BRIDGE_PREV:
                right_br = right_br->prev;
                result.push_back(right_br);
                break;
            case RIGHT_BRIDGE_NEXT:
                result.push_back(right_br);
                right_br = right_br->next;
                break;
        }
    }
    buildBridges(result, left_br, right_br, x);

    return result;
}

template <typename T>
void ConvexHull3D<T>::buildBridges(std::vector<Point3DChan<T> *> & data,
                                   Point3DChan<T> *& left_br, Point3DChan<T> *& right_br, T x) {

    left_br->next = right_br;
    right_br->prev = left_br;

    for (auto cur = data.rbegin(); cur != data.rend(); ++cur) {
        auto cur_point = *cur;

        if (left_br->x < cur_point->x && cur_point->x < right_br->x) {
            left_br->next = cur_point;
            right_br->prev = cur_point;

            cur_point->prev = left_br;
            cur_point->next = right_br;

            if (cur_point->x <= x) {
                left_br = cur_point;
            } else {
                right_br = cur_point;
            }
        } else {
            cur_point->updateLinks();
            if (cur_point == left_br) {
                left_br = left_br->prev;
            }
            if (cur_point == right_br) {
                right_br = right_br->next;
            }
        }
    }
}
