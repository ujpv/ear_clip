#pragma once

#include <array>
#include <vector>
#include <list>
#include <tuple>

namespace ear_clip {

struct Point {
    double x, y;
    [[nodiscard]] auto tie() const { return std::tie(x, y); }
    bool operator==(const Point& other) const { return tie() == other.tie(); };
    bool operator<(const Point& other) const { return tie() < other.tie(); }
};

using Triangle = std::array<Point, 3>;
using Ring = std::list<Point>;

void enableTrace(bool enable);

std::vector<Triangle> triangulate(Ring ring);

namespace details {

enum class Direction {
    CWISE,
    CCWISE,
    NO_AREA
};

Direction ringDirection(const Ring& ring);
Direction triangleDirection(const Triangle& triangle);
Ring normalizeRing(Ring ring);

bool intersects(Point a, Point b, Point c, Point d);
Point intersection(Point a, Point b, Point c, Point d);
double angleRad(Point a, Point b, Point c);
bool pointInTriangle(const Triangle& t, Point p);

}

}