#include "ear_clip.h"

#include <algorithm>
#include <exception>
#include <optional>
#include <map>
#include <numeric>
#include <cmath>
#include <iostream>

#include "../tests/test_util.h"

namespace ear_clip {

namespace {

double signedArea(const Triangle& t)
{
    const Point& a = t[0];
    const Point& b = t[1];
    const Point& c = t[2];
    auto area = (c.y - b.y) * (a.x - c.x) - (c.x - b.x) * (a.y - c.y);
    return area;
}

}

std::vector<Triangle> triangulate(Polygon polygon)
{
    std::cerr << "Src polygon: " << polygon << '\n';
    polygon = details::normalizePolygon(std::move(polygon));
    std::cerr << "Norm polygon: " << polygon << '\n';
    polygon = details::selfIntersect(std::move(polygon));
    std::cerr << "self intersected polygon: " << polygon << '\n';
    polygon = details::normalizePolygon(std::move(polygon));
    std::cerr << "Norm polygon: " << polygon << '\n';

    if (polygon.empty())
        return {};

    if (polygon.size() < 3)
        throw std::invalid_argument("It's not a polygon");

    auto rot = details::ringDirection(polygon);
    auto nextIt = [&polygon](const auto& it) {
        auto next = std::next(it);
        if (next == polygon.end())
            next = polygon.begin();
        return next;
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
        if (*a == *c) {
            counter = 0;
            polygon.erase(b);
            polygon.erase(c);
            continue;
        }

        Triangle t{*a, *b, *c};
        if (signedArea(t) == 0) {
            counter = 0;
            polygon.erase(b);
            continue;
        }

        bool isEar = triangleDirection(t) == rot;
        if (isEar) {
            for (auto vIt = nextIt(c); vIt != a; vIt = nextIt(vIt)) {
                Point& p = *vIt;
                if (pointInTriangle(t, p)) {
                    isEar = false;
                    break;
                }
            }
        }

        if (isEar) {
            result.push_back(t);
            polygon.erase(b);
            counter = 0;
        }
        a = nextIt(a);
    }

    return result;
}

namespace details {

bool pointInTriangle(const Triangle& t, Point p)
{
    if (p == t[0] || p == t[1] || p == t[2])
        return false;

    const Point& a = t[0];
    const Point& b = t[1];
    const Point& c = t[2];
    return (c.x - p.x) * (a.y - p.y) - (a.x - p.x) * (c.y - p.y) >= 0 &&
           (a.x - p.x) * (b.y - p.y) - (b.x - p.x) * (a.y - p.y) >= 0 &&
           (b.x - p.x) * (c.y - p.y) - (c.x - p.x) * (b.y - p.y) >= 0;
}

Direction ringDirection(const Polygon& ring)
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

Polygon normalizePolygon(Polygon polygon)
{
    if (polygon.empty())
        return polygon;
    if (polygon.size() < 3)
        throw std::invalid_argument("It's not a polygon");

    if (polygon.back() == polygon.front())
        polygon.pop_back();

    polygon.unique();
    if (details::ringDirection(polygon) != Direction::CCWISE) {
        std::reverse(polygon.begin(), polygon.end());
    }

    return polygon;
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

Ring selfIntersect(Ring ring)
{
    if (ring.size() < 3)
        return ring;

    std::vector<Point> nodes;
    nodes.reserve(ring.size());
    std::map<Point, size_t> pointToNode;
    auto getPointInd = [&](Point p) {
        if (auto it = pointToNode.find(p); it != pointToNode.end()) {
            return it->second;
        }

        auto ind = nodes.size();
        nodes.push_back(p);
        pointToNode[p] = ind;
        return ind;
    };
    for (auto p: ring)
        getPointInd(p);

    std::vector<std::optional<std::pair<size_t, size_t>>> edges;
    for (auto b = ring.begin(), e = std::next(ring.begin()); e != ring.end(); b++, e++) {
        auto edge = std::make_pair(getPointInd(*b), getPointInd(*e));
        edges.emplace_back(edge);
    }
    auto args = std::make_pair(
        getPointInd(ring.front()), getPointInd(ring.back()));
    edges.emplace_back(args);

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

    std::vector<Point> points;
    for(const auto& [edge, splitPoints]: edgeToSplitPoints) {
        points.clear();
        points.push_back(nodes[edges[edge]->first]);
        points.push_back(nodes[edges[edge]->second]);
        for (auto p: splitPoints)
            points.push_back(p);
        std::sort(points.begin(), points.end());

        edges[edge] = std::nullopt;

        for (auto b = points.begin(), e = std::next(points.begin()); e != points.end(); b++, e++) {
            edges.emplace_back(std::make_pair(getPointInd(*b), getPointInd(*e)));
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

    size_t startPoint = getPointInd(mostLeft);
    {   // setup traverse order
        auto fakeNode = mostLeft; // first node for dfs
        fakeNode.x *= 0.1; // shift left

        std::vector<double> angles(nodes.size());
        std::vector<char> visited(nodes.size());
        std::vector<std::pair<size_t, Point>> stack{{{startPoint, fakeNode}}};
        while (!stack.empty()) {
            auto[nodeId, prevPoint] = stack.back();
            auto nodePoint = nodes[nodeId];
            stack.pop_back();
            if (visited[nodeId])
                continue;
            visited[nodeId] = true;

            auto& neighbours = graph[nodeId];
            for (auto [toId, _]: neighbours) {
                angles[toId] = angleRad(prevPoint, nodePoint, nodes[toId]);
            }

            std::sort(neighbours.begin(), neighbours.end(),
                      [&angles](auto l, auto r) {
                          return angles[l.first] < angles[r.first];
                      });

            std::reverse(neighbours.begin(), neighbours.end());
            for (auto [toId, _]: neighbours) {
                stack.emplace_back(toId, nodePoint);
            }
            std::reverse(neighbours.begin(), neighbours.end());
        }
    }

    std::vector<size_t> traverseOrder;
    traverseOrder.reserve(nodes.size());
    { // traverse
        std::vector<std::pair<size_t, size_t>> stack; // (nodeId, edgeId)
        stack.push_back(graph[startPoint].front());
        while (!stack.empty()) {
            auto [nodeId, edgeId] = stack.back();
            stack.pop_back();
            if (!edges[edgeId]) {
                continue;
            }
            edges[edgeId] = std::nullopt;
            traverseOrder.push_back(nodeId);
            auto & v = graph[nodeId];
            for (auto [toId, toEdgeId]: v) {
                if (edges[toEdgeId]) {
                    stack.emplace_back(toId, toEdgeId);
                    break;
                }
            }
        }
    }

    ring.clear();
    for (auto id: traverseOrder)
        ring.push_back(nodes[id]);

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