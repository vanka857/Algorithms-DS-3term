//E. Гадание
//        Ограничение времени	2 секунды
//        Ограничение памяти	256Mb
//Ввод	стандартный ввод или input.txt
//        Вывод	стандартный вывод или output.txt
//        Арсений открыл эзотерический салон в свои снах на планете Ка-Пэкс.
//        У Арсения всё будет хорошо. А вот у его клиентов — не факт.
//        Если хотя бы какие-нибудь их палочки в гадании "Мокусо Дзэй" пересеклись,
//        то день точно сложится удачно. А если нет — то стоит ждать беды.
//        Чтобы точнее сказать, что конкретно пойдет хорошо в этот день, нужно знать,
//        какие именно палочки пересекаются. Помогите Арсению узнать,
//        как пройдет день у его клиентов.
//
//Формат ввода
//  Палочка представляется как отрезок прямой.
//  В первой строке входного файла записано число N (1 ≤ N ≤ 125 000) — количество палочек в гадании.
//  Следующие N строк содержат описания палочек: (i + 1)-я строка содержит
//  координаты концов i-й палочки — целые числа x1, y1, x2, y2 (-10 000 ≤ x1, y1, x2, y2 ≤ 10 000).
//
//Формат вывода
//  В первой строке выходного файла выведите слово "YES", если день сложится удачно.
//  Вторая строка должна содержать числа i и j — номера палочек, которые пересекаются.
//  Если день пройдет плохо, выведите в единственной строке выходного файла "NO".

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include "geometry2D.cpp"

template<typename T>
// expected:
// T - integer or floating point type
class Segment {
public:
    Point<T> p;
    Point<T> q;

    size_t id = 0;

    Segment() = default;
    Segment(const Point<T> & p, const Point<T> & q, size_t id) : p(p), q(q), id(id) {}

    [[nodiscard]] double getY(const T & x) const {
        if (is_equal(p.x, q.x))  {
            return p.y;
        }
        // static_cast для расчетв в формате с плавающей точкой
        return p.y + (q.y - p.y) * (x - p.x) / static_cast<double >(q.x - p.x);
    }

    [[nodiscard]] bool segmCompare (const Segment & rhs, const T x) const {
        if (!is_equal(getY(x), rhs.getY(x))) {
            return getY(x) < rhs.getY(x);
        }
        return id < rhs.id;
    }
};

template<typename T>
bool intersects(const Segment<T> & lhs, const Segment<T> & rhs) {
    return intersects(lhs.p, lhs.q, rhs.p, rhs.q);
}

enum class EventType{
    BEGIN,
    END
};

template<typename T>
class Event {
public:
    size_t id = 0;
    EventType event_type = EventType::BEGIN;

    T x;

    Event() = default;
    Event(size_t id, EventType event_type, const T & x) : id(id), event_type(event_type), x(x) {}

    bool operator < (const Event & rhs) const {
        if (!is_equal(x, rhs.x)) {
            return x < rhs.x;
        }
        return event_type < rhs.event_type;
    }
};

template<class Container>
void print(const Container & container, std::ostream & out = std::cout) {
    for (const auto & i : container) {
        // TODO replace id
        out << i.id << " ";
    }
    out << std::endl;
}

template<class Container>
typename Container::iterator my_prev(const Container & container, const typename Container::iterator & it) {
    if (it == container.begin()) {
        return container.end();
    }
    return std::prev(it);
}

template<class Container>
typename Container::iterator my_next(const Container & container, const typename Container::iterator & it) {
    if (it == container.end()) {
        return container.end();
    }
    return std::next(it);
}

template<typename T>
std::pair<ssize_t, ssize_t>
firstIntersections(const std::vector<Segment<T>> & segments) {

    std::vector<Event<T>> events;
    events.reserve(segments.size() * 2);

    for (const auto & segment : segments) {
        events.emplace_back(segment.id, EventType::BEGIN, segment.p.x);
        events.emplace_back(segment.id, EventType::END, segment.q.x);
    }
    std::sort(events.begin(), events.end());

    T x;

    auto SegmentLess = [&x](const Segment<T> & lhs,  const Segment<T> & rhs) {
        return lhs.segmCompare(rhs, x);
    };
    std::set<Segment<T>, decltype(SegmentLess)> currentSegments(SegmentLess);

    std::vector<typename std::set<Segment<T>>::iterator> set_it(segments.size());

    for (const auto & event : events) {
        x = event.x;
        auto & cur = segments.at(event.id);
        auto cur_it = set_it[cur.id];

        switch (event.event_type) {
            case EventType::BEGIN: {
                auto next = currentSegments.lower_bound(cur);
                auto prev = my_prev(currentSegments, next);

                if (prev != currentSegments.end() && intersects(cur, *prev))
                    return {cur.id, prev->id};
                if (next != currentSegments.end() && intersects(cur, *next))
                    return {cur.id, next->id};

                set_it[cur.id] = currentSegments.insert(next, cur);
                break;
            }
            case EventType::END: {
                auto next = my_next(currentSegments, cur_it);
                auto prev = my_prev(currentSegments, cur_it);

                if (next != currentSegments.end() && prev != currentSegments.end() && intersects(*next, *prev))
                    return {next->id, prev->id};

                currentSegments.erase(cur_it);
                break;
            }
        }
    }
    return {-1, -1};
}

int main() {
    using T = int64_t;

    size_t N;
    std::cin >> N;

    std::vector<Segment<T>> segments;
    segments.reserve(N);

    for (size_t i = 0; i < N; ++i) {
        Point<T> p{}, q{};
        std::cin >> p >> q;

        auto res = std::minmax(p, q, xLess_yGreater<T>());

        segments.emplace_back(res.first, res.second, i);
    }

    auto res = firstIntersections(segments);
    // lets order pair
    auto [first, second] = std::minmax(res.first, res.second);

    if (first == -1 || second == -1) {
        std::cout << "NO" << std::endl;
    }
    else {
        // remember that segments are numerated from0 to N
        std::cout << "YES\n" << first + 1 << " " << second + 1 << std::endl;
    }
}

//3
//0 3 0 4
//0 1 0 2
//0 0 0 1

//3
//0 0 2 2
//0 2 2 1
//0 1 2 1
