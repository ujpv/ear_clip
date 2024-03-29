#pragma once

#include <array>
#include <vector>
#include <list>
#include <tuple>

namespace ear_clip {

struct Point {
  double x, y;
  [[nodiscard]] auto tie() const;
  bool operator==(const Point &other) const;
  bool operator<(const Point &other) const;
};

using Triangle = std::array<Point, 3>;
using Ring = std::list<Point>;

void enableTrace(bool enable);

std::vector<Triangle> triangulate(Ring ring);

namespace details {

enum class VertexOrder {
  CLOCKWISE,
  C_CLOCKWISE,
  NO_AREA
};

VertexOrder vertexOrder(const Ring &ring);
VertexOrder vertexOrder(const Triangle &triangle);
Ring normalizeRing(Ring ring);

bool intersects(Point a, Point b, Point c, Point d);
Point intersection(Point a, Point b, Point c, Point d);
double angleRad(Point a, Point b, Point c);
bool pointInTriangle(const Triangle &t, Point p);

} // namespace details

} // namespace ear_clip