#include "geometry2D.cpp"
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

const static double INF = 1e9;

template<typename T>
class Point3D {
public:
    T x, y, z;

    size_t id;

    Point3D(const T & x, const T & y, const T & z, size_t id) : x(x), y(y), z(z), id(id) {}

    void rotate(double angle) {
        double new_z = z * cos(angle) + y * sin(angle);
        double new_y = -z * sin(angle) + y * cos(angle);
        z = new_z;
        y = new_y;

        double new_x = x * cos(angle) + z * sin(angle);
        new_z = -x * sin(angle) + z * cos(angle);
        x = new_x;
        z = new_z;

        new_x = x * cos(angle) + y * sin(angle);
        new_y = -x * sin(angle) + y * cos(angle);
        x = new_x;
        y = new_y;
    }
};

template<typename T>
class Vertex : public Point3D<T> {
public:
    Vertex<T> * prev = nullptr;
    Vertex<T> * next = nullptr;

    Vertex(const T & x, const T & y, const T & z, size_t id) : Point3D<T>(x, y, z, id) {}

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
Point3D<T> readPoint3D(std::istream & in, size_t id) {
    T x, y, z;
    in >> x >> y >> z;
    return {x, y, z, id};
}

template<typename T>
struct Face {
    std::vector<Vertex<T>> vertices;

    Face() = default;
    explicit Face(const std::vector<Vertex<T> > & points) : vertices(points) {}
    Face(Vertex<T> v1, Vertex<T> v2, Vertex<T> v3) {
        vertices.emplace_back(v1);
        vertices.emplace_back(v2);
        vertices.emplace_back(v3);
    }

    [[nodiscard]] std::string toString() const;

    void getLessOrder();
};

template<typename T>
std::string Face<T>::toString() const {
    std::string s = "3";
    return s + " " + std::to_string(vertices[0].id) + " " + std::to_string(vertices[1].id) + " " + std::to_string(vertices[2].id);
}

template<typename T>
void Face<T>::getLessOrder() {
    if (vertices[1].id < vertices[0].id  && vertices[1].id < vertices[2].id) {
        std::swap(vertices[0], vertices[1]);
        std::swap(vertices[1], vertices[2]);
    } else if (vertices[2].id < vertices[0].id && vertices[2].id < vertices[1].id) {
        std::swap(vertices[1], vertices[2]);
        std::swap(vertices[0], vertices[1]);
    }
}

template<typename T>
class ConvexHull3DClass {
private:
    std::vector<Vertex<T> > points;

public:
    static bool xLess_zGreater(Point3D<T> lhs, Point3D<T> rhs) {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.z > rhs.z);
    }

    static bool zLess_xGreater(Point3D<T> lhs, Point3D<T> rhs) {
        return lhs.z < rhs.z || (lhs.z == rhs.z && lhs.x > rhs.x);
    }

    explicit ConvexHull3DClass(const std::vector<Vertex<T> > & points) : points(points) {}
    std::vector<Face<T>> hull() const; // return ConvexHull as faces

    std::vector<Face<T>> lowerHull(std::function<bool (Vertex<T>, Vertex<T>)> less = xLess_zGreater) const; // return Lower ConvexHull as faces
    std::vector<Face<T>> upperHull(std::function<bool (Vertex<T>, Vertex<T>)> less = xLess_zGreater) const; // return Upper ConvexHull as faces

private:

    enum EventType {
        LEFT,
        RIGHT,
        LEFT_BRIDGE_PREV,
        LEFT_BRIDGE_NEXT,
        RIGHT_BRIDGE_PREV,
        RIGHT_BRIDGE_NEXT,
    };

    void sortPoints(std::vector<Vertex<T>> & source, std::function<bool (Vertex<T>, Vertex<T>)> less) const;
    static std::vector<Vertex<T> > reflected(typename std::vector<Vertex<T>>::iterator begin, typename std::vector<Vertex<T>>::iterator end) {
        std::vector<Vertex<T> > result;

        for (auto it = begin; it < end; ++it) {
            auto pt = *it;
            Vertex<T> v(pt.x, pt.y, - pt.z, pt.id);
            v.prev = nullptr;
            v.next = nullptr;
            result.push_back(v);
        }

        return result;
    }

    static std::vector<Vertex<T> *> hull_raw(std::vector<Vertex<T> *> & pts, size_t from, size_t to) ; // return ConvexHull of part [begin, end) as ordered vector of ptr to points
    static std::vector<Vertex<T> *> merge(const std::vector<Vertex<T> *> & left, const std::vector<Vertex<T> *> & right, Vertex<T> * left_br, Vertex<T> * right_br); // merge two ConvexHulls

    static void buildBridges(std::vector<Vertex<T> *> & data, Vertex<T> * left_br, Vertex<T> * right_br, T x);
    static std::pair<Vertex<T> *, Vertex<T> *> findBridge(Vertex<T> * left_br, Vertex<T> * right_br);

    static T turnTime(Vertex<T> * a, Vertex<T> * b, Vertex<T> * c);
    static bool isRightTurn(Vertex<T> * a, Vertex<T> * b, Vertex<T> * c);

    static void pushFaces(std::vector<Face<T>> & faces, const std::vector<Vertex<T> *> & points, bool reflected=false);
    static std::vector<Vertex<T> *> links(const std::vector<Vertex<T>> & points);
};

template <typename T>
void ConvexHull3DClass<T>::pushFaces(std::vector<Face<T>> & faces, const std::vector<Vertex<T> *> & points, bool reflected) {
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
std::vector<Vertex<T> *> ConvexHull3DClass<T>::links(const std::vector<Vertex<T>> & points) {
    std::vector<Vertex<T> *> result;

    // TODO fix memory leak
    for (auto & p : points) {
        auto ptr = new Vertex<T>(p);
        result.push_back(ptr);
    }

    return result;
}

template<typename T>
std::vector<Face<T>> ConvexHull3DClass<T>::lowerHull(std::function<bool (Vertex<T>, Vertex<T>)> less) const {
    std::vector<Face<T>> result;

    auto sorted = points;
    sortPoints(sorted, less);

    auto sorted_links = links(sorted);
    auto lch = hull_raw(sorted_links, 0, sorted_links.size());
    // add faces from lch (lower convex hull)
    pushFaces(result, lch, false);

    return result;
}

template<typename T>
std::vector<Face<T>> ConvexHull3DClass<T>::upperHull(std::function<bool (Vertex<T>, Vertex<T>)> less) const {
    std::vector<Face<T>> result;

    auto sorted = points;
    sortPoints(sorted, less);

    auto points_reflected = reflected(sorted.begin(), sorted.end());

    auto reflected_links = links(points_reflected);
    auto uch = hull_raw(reflected_links, 0, reflected_links.size());
    // add faces from uch (upper convex hull)
    pushFaces(result, uch, true);

    return result;
}

template<typename T>
std::vector<Face<T>> ConvexHull3DClass<T>::hull() const {
    auto lch = lowerHull();
    auto uch = upperHull();

    std::move(uch.begin(), uch.end(), std::back_inserter(lch));

    return lch;
}


template<typename T>
void ConvexHull3DClass<T>::sortPoints(std::vector<Vertex<T>> & source, std::function<bool (Vertex<T>, Vertex<T>)> less) const {
    std::sort(source.begin(), source.end(), less);
}

template<typename T>
std::vector<Vertex<T> *>
ConvexHull3DClass<T>::hull_raw(std::vector<Vertex<T> *> & pts, size_t from, size_t to) {

    if (from + 1 == to) { // n == 1
        std::vector<Vertex<T> *> result;
        result.emplace_back(pts[from]);
        return result;
    }

    size_t mid = from + static_cast<int>((to - from) / 2);

    auto r1 = hull_raw(pts, from, mid);
    auto r2 = hull_raw(pts, mid, to);

    return  merge(r1, r2, pts[mid - 1], pts[mid]);
}

template <typename T>
T ConvexHull3DClass<T>::turnTime(Vertex<T> * a, Vertex<T> * b, Vertex<T> * c) {
    if (a == nullptr || b == nullptr || c == nullptr) {
        return INF;
    }
    Vector<T> ab_y({a->x, a->y}, {b->x, b->y});
    Vector<T> bc_y({b->x, b->y}, {c->x, c->y});

    Vector<T> ab_z({a->x, a->z}, {b->x, b->z});
    Vector<T> bc_z({b->x, b->z}, {c->x, c->z});

    T temp = crossProduct(ab_y, bc_y);
    return is_equal(temp,  static_cast<T>(0)) ? INF : crossProduct(ab_z, bc_z) / temp;
}

template <typename T>
bool ConvexHull3DClass<T>::isRightTurn(Vertex<T> * a, Vertex<T> * b, Vertex<T> * c) {
    if (a == nullptr || b == nullptr || c == nullptr) {
        return false;
    }

    Vector<T> ab_y({a->x, a->y}, {b->x, b->y});
    Vector<T> bc_y({b->x, b->y}, {c->x, c->y});

    return is_less_equal(crossProduct(ab_y, bc_y), static_cast<T>(0));
}

template<typename T>
std::pair<Vertex<T> *, Vertex<T> *>
ConvexHull3DClass<T>::findBridge(Vertex<T> * left_br,
                                 Vertex<T> * right_br) {

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
std::vector<Vertex<T> *>
ConvexHull3DClass<T>::merge(const std::vector<Vertex<T> *> & left,
                            const std::vector<Vertex<T> *> & right,
                            Vertex<T> * left_br_, Vertex<T> * right_br_) {

    std::vector<Vertex<T> *> result;

    auto x = left_br_->x;
    auto bridge = findBridge(left_br_, right_br_);

    auto * left_br = bridge.first;
    auto * right_br = bridge.second;

    T time = -INF;

    auto l_it = left.begin();
    auto r_it = right.begin();


    while (true) {
        T min_time = INF;

        T temp;
        EventType event;

        Vertex<T> * l;
        Vertex<T> * r;

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

        if (is_equal(min_time, INF)) break;

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
void ConvexHull3DClass<T>::buildBridges(std::vector<Vertex<T> *> & data,
                                        Vertex<T> * left_br, Vertex<T> * right_br, T x) {

    left_br->next = right_br;
    right_br->prev = left_br;

    for (auto cur = data.rbegin(); cur != data.rend(); ++cur) {
        // TODO may be cur_point = *cur is same
        auto cur_point = &*(*cur);

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
