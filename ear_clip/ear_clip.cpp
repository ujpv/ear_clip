#include "ear_clip.h"

#include <algorithm>
#include <exception>
#include <optional>
#include <map>
#include <numeric>
#include <cmath>
#include <iostream>

namespace ear_clip {

namespace {

std::vector<Point> nodes;
std::map<Point, size_t> pointToNode;
auto getPointInd = [](Point p) {
    if (auto it = pointToNode.find(p); it != pointToNode.end()) {
        return it->second;
    }

    auto ind = nodes.size();
    nodes.push_back(p);
    pointToNode[p] = ind;
    return ind;
};

std::ostream& operator<<(std::ostream& s, const Point& point) {
    s << point.x << ' ' << point.y;
    return s;
}

std::ostream& operator<<(std::ostream& s, const Ring& polygon) {
    for (auto p: polygon)
        s << p.x << ' ' << p.y << ' ';

    return s;
}

template <class T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& v) {
    for (auto i: v)
        s << i << ' ';

    return s;
}

double signedArea(const Triangle& t)
{
    const Point& a = t[0];
    const Point& b = t[1];
    const Point& c = t[2];
    auto area = (c.y - b.y) * (a.x - c.x) - (c.x - b.x) * (a.y - c.y);
    return area;
}

}

std::vector<Triangle> triangulate(Ring polygon)
{
    std::cerr << "Src  polygon: " << polygon << '\n';
    polygon = details::normalizeRing(std::move(polygon));
    std::cerr << "Norm polygon: " << polygon << '\n';

    if (polygon.empty())
        return {};

    if (polygon.size() < 3)
        throw std::invalid_argument("It's not a polygon");

    auto ringRotation = details::ringDirection(polygon);
    auto nextIt = [&polygon](const auto& it) {
        auto next = std::next(it);
        if (next == polygon.end())
            next = polygon.begin();
        return next;
    };
    auto prevIt = [&polygon](const auto& it) {
        if (it == polygon.begin())
            std::prev(polygon.end());
        return std::prev(it);
    };

    std::vector<Triangle> result;
    result.reserve(polygon.size() - 2);
    using namespace details;
    auto a = polygon.begin();
    size_t counter = 0;
    while (polygon.size() > 2 && counter < polygon.size()) {
        counter++;
        auto b = nextIt(a);
        auto c = nextIt(b);
        std::cerr << "Triangle: " << getPointInd(*a) << '-' << getPointInd(*b) << '-' << getPointInd(*c) << '\n';

        Triangle t{*a, *b, *c};
        if (signedArea(t) == 0) {
            std::cerr << "Zero area. Skip\n";
            a = nextIt(a);
            continue;
        }

        bool isEar = triangleDirection(t) == ringRotation;
        if (isEar) {
            std::cerr << "Ear rotation. ";
            for (auto vIt = nextIt(c); vIt != a; vIt = nextIt(vIt)) {
                Point& p = *vIt;
                if (pointInTriangle(t, p)) {
                    isEar = false;
                    std::cerr << "Contains other points. ";
                    break;
                }
            }
        }

        auto removeEmptyLoops = [&](auto a) {
            bool changed = true;
            while (changed && polygon.size() > 3) {
                changed = false;
                auto b = nextIt(a); auto c = nextIt(b);
                if (*a == *c) {
                    polygon.erase(b);
                    polygon.erase(c);
                    changed = true;
                }
                if (polygon.size() < 3)
                    return a;
                b = prevIt(a); c = nextIt(a);
                if (*b == *c) {
                    polygon.erase(a);
                    polygon.erase(c);
                    a = b;
                    changed = true;
                }
                if (polygon.size() < 3)
                    return a;
                b = prevIt(a); c = prevIt(b);
                if (*c == *a) {
                    polygon.erase(a);
                    polygon.erase(b);
                    a = c;
                    changed = true;
                }
            }

            return a;
        };

        if (isEar) {
            std::cerr << "clip!\n";
            result.push_back(t);
            polygon.erase(b);
            a = removeEmptyLoops(a);
            a = nextIt(a);
            counter = 0;
        } else {
            std::cerr << "skip\n";
            a = nextIt(a);
        }
    }

    return result;
}

namespace details {


Direction ringDirection(const Ring& ring)
{
    if (ring.size() < 3)
        throw std::invalid_argument("Ring has less than 3 points");

    auto highest = std::max_element(ring.begin(), ring.end(),
        [](auto l, auto r) { return l.y < r.y; });

    auto prev = highest == ring.begin() ?
                std::prev(ring.end(), 1) : std::prev(highest);

    auto next = std::next(highest);
    if (next == ring.end())
        next = ring.begin();

    Triangle triangle = {*prev, *highest, *next};
    return triangleDirection(triangle);
}

namespace {

double signedArea(const Point& a, const Point& b, const Point& c)
{
    auto area = (c.y - b.y) * (a.x - c.x) - (c.x - b.x) * (a.y - c.y);
    return area;
}

}

Direction triangleDirection(const ear_clip::Triangle& triangle)
{
    const Point& a = triangle[0];
    const Point& b = triangle[1];
    const Point& c = triangle[2];
    double area = signedArea(a, b, c);
    if (area == 0.0)
        throw std::invalid_argument("Invalid triangle");

    return area > 0 ? Direction::CWISE : Direction::CCWISE;
}

bool pointInTriangle(const Triangle& t, Point p)
{
    if (p == t[0] || p == t[1] || p == t[2])
        return false;

    double d1 = signedArea(p, t[0], t[1]);
    double d2 = signedArea(p, t[1], t[2]);
    double d3 = signedArea(p, t[2], t[0]);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

bool intersects(Point a, Point b, Point c, Point d)
{
    return signedArea(a, b, c) * signedArea(a, b, d) < 0 &&
           signedArea(c, d, a) * signedArea(c, d, b) < 0;
}

Point intersection(Point a, Point b, Point c, Point d)
{
    double a1 = b.y - a.y;
    double b1 = a.x - b.x;
    double c1 = a1 * a.x + b1 * a.y;

    double a2 = d.y - c.y;
    double b2 = c.x - d.x;
    double c2 = a2 * c.x + b2 * c.y;

    double det = a1 * b2 - a2 * b1;
    if (det == 0)
        throw std::logic_error("There are no intersection");

    double x = (b2 * c1 - b1 * c2) / det;
    double y = (a1 * c2 - a2 * c1) / det;
    return {x, y};
}

Ring normalizeRing(Ring ring)
{
    if (ring.empty())
        return ring;
    else if (ring.size() == 1)
        throw std::invalid_argument("It's not a polygon");
    else if (ring.back() == ring.front()) {
        ring.pop_back();
        if (ring.size() == 1)
            throw std::invalid_argument("It's not a polygon");
    }
    nodes.reserve(ring.size());

    // for debug. remove it
    for (auto p: ring)
        getPointInd(p);

    std::vector<std::optional<std::pair<size_t/*from*/, size_t/*to*/>>> edges;
    for (auto b = ring.begin(), e = std::next(ring.begin()); e != ring.end(); b++, e++) {
        auto edge = std::make_pair(getPointInd(*b), getPointInd(*e));
        if (edge.first == edge.second)
            continue;

        edges.emplace_back(edge);
    }
    auto args = std::make_pair(
        getPointInd(ring.front()), getPointInd(ring.back()));
    edges.emplace_back(args);

    std::cerr << "Building graph:\n";
    for (const auto e: edges) {
        std::cerr << "Edge: " <<  e->first << '-' << e->second << '\n';
    }

    std::map<size_t, std::vector<Point>> edgeToSplitPoints;
    for (size_t i = 0; i < edges.size(); ++i) {
        for (size_t j = i + 1; j < edges.size(); ++j) {
            auto a = nodes[edges[i]->first];
            auto b = nodes[edges[i]->second];
            auto c = nodes[edges[j]->first];
            auto d = nodes[edges[j]->second];
            if (intersects(a, b, c, d)) {
                auto p = intersection(a, b, c, d);
                getPointInd(p); // store to nodes storage
                edgeToSplitPoints[i].push_back(p);
                edgeToSplitPoints[j].push_back(p);
            }
        }
    }

    std::cerr << "Nodes:\n";
    for (size_t i = 0; i < nodes.size(); ++i) {
        std::cerr << i << ": (" << nodes[i] << ")\n";
    }

    std::cerr << "Splitting edges:\n";
    std::vector<Point> points;
    for(const auto& [edge, splitPoints]: edgeToSplitPoints) {
        points.clear();
        points.push_back(nodes[edges[edge]->first]);
        points.push_back(nodes[edges[edge]->second]);
        for (auto p: splitPoints)
            points.push_back(p);
        std::sort(points.begin(), points.end());

        edges[edge] = std::nullopt;

        std::cerr << edges[edge]->first << '-' << edges[edge]->second << " to:\n";
        for (auto b = points.begin(), e = std::next(points.begin()); e != points.end(); b++, e++) {
            edges.emplace_back(std::make_pair(getPointInd(*b), getPointInd(*e)));
            std::cerr << edges.back()->first << '-' << edges.back()->second << '\n';
        }
    }

    Point mostLeft = nodes[edges.front()->first]; // first node for dfs
    std::vector<std::vector<std::pair<size_t, size_t>>> graph(nodes.size()); // planar(toId, edgeId)
    for (size_t i = 0; i < edges.size(); ++i) {
        auto e = edges[i];
        if (!e)
            continue;
        graph[e->first].emplace_back(e->second, i);
        graph[e->second].emplace_back(e->first, i);
        mostLeft = std::min(mostLeft, nodes[e->first]);
        mostLeft = std::min(mostLeft, nodes[e->second]);
    }

    size_t startPointId = getPointInd(mostLeft);
    std::cerr << "Start node: " << startPointId << '\n';
    {   // setup traverse order
        auto fakeNode = mostLeft; // first node for dfs
        fakeNode.x = std::numeric_limits<double>::lowest(); // shift left

        std::vector<double> angles(nodes.size());
        std::vector<char> visited(nodes.size());
        std::vector<std::pair<size_t/*node to*/, Point /*prev point*/>> stack{
            {{startPointId, fakeNode}}};

        std::vector<size_t> debugOrder;
        while (!stack.empty()) {
            auto[nodeId, prevPoint] = stack.back();
            auto nodePoint = nodes[nodeId];
            stack.pop_back();
            if (visited[nodeId])
                continue;
            visited[nodeId] = true;
            debugOrder.push_back(nodeId);

            auto& neighbours = graph[nodeId];
            for (auto [toId, _]: neighbours) {
                angles[toId] = angleRad(prevPoint, nodePoint, nodes[toId]);
            }

            std::sort(neighbours.begin(), neighbours.end(),
                      [&angles](auto l, auto r) {
                          return angles[l.first] < angles[r.first];
                      });

            for (auto [toId, _]: neighbours) {
                stack.emplace_back(toId, nodePoint);
            }
        }

        std::cerr << "Debug order: " << debugOrder << '\n';
    }

    std::vector<size_t> traverseOrder;
    traverseOrder.reserve(nodes.size());
    { // traverse
        std::vector<std::pair<size_t, size_t>> stack; // (nodeId, edgeId)
        stack.push_back(graph[startPointId].back());
        while (!stack.empty()) {
            auto [nodeId, edgeId] = stack.back();
            stack.pop_back();
            if (!edges[edgeId]) {
                continue;
            }
            edges[edgeId] = std::nullopt;
            traverseOrder.push_back(nodeId);
            auto& v = graph[nodeId];
            while (!v.empty()) {
                auto [toId, toEdgeId] = v.back();
                v.pop_back();
                if (!edges[toEdgeId])
                    continue;

                stack.emplace_back(toId, toEdgeId);
                break;
            }
        }
    }

    ring.clear();
    for (auto id: traverseOrder)
        ring.push_back(nodes[id]);

    std::cerr << "Traverse order: " << traverseOrder << '\n';

    return ring;
}

double angleRad(Point a, Point b, Point c)
{
    a.x -= b.x;
    a.y -= b.y;
    c.x -= b.x;
    c.y -= b.y;
    double angle = atan2(a.y, a.x) - atan2(c.y, c.x);
    if (angle < 0)
        angle += 2 * M_PI;

    return angle;
}

}

}