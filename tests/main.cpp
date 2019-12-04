#include <iostream>
#include <algorithm>
#include <map>
#include <tuple>
#include <cmath>

#include "ear_clip.h"
#include "test_util.h"

namespace ec = ear_clip;
namespace ecd = ear_clip::details;


bool testTriangleRotation(ec::Triangle t, ecd::Direction dir_tar)
{
    std::cout << "Test triangle rotation. Expected: " << dir_tar << '\n';
    bool failed = false;
    for (size_t i = 0; i < t.size(); ++i) {
        std::rotate(t.begin(), t.begin() + 1, t.end());
        std::cout << "Test triangle " << t << ": ";
        auto dir = ecd::triangleDirection(t);
        bool ok = dir == dir_tar;
        std::cout << (ok ? "OK" : "Failed") << '\n';
        failed |= !ok;
    }
    return failed;
}

bool testRingRotation(ec::Ring r, ecd::Direction dir_tar)
{
    std::cout << "Test ring rotation. Expected: " << dir_tar << '\n';
    bool failed = false;
    for (size_t i = 0; i < r.size(); ++i) {
        std::rotate(r.begin(), std::next(r.begin()), r.end());
        std::cout << "Test triangle " << r << ": ";
        auto dir = ecd::ringDirection(r);
        bool ok = dir == dir_tar;
        std::cout << (ok ? "OK" : "Failed") << '\n';
        failed |= !ok;
    }
    return failed;
}

void normalize(std::vector<ec::Triangle>& ts) {
    for (auto& t: ts) {
        std::sort(t.begin(), t.end());
    }
    std::sort(ts.begin(), ts.end());
}

bool testTriangWithRotation(ec::Ring r, std::vector<ec::Triangle> expected)
{
    normalize(expected);
    std::cout << "Test triangulation with rotation. Expected: " << expected << '\n';
    bool failed = false;
    for (size_t i = 0; i < r.size(); ++i) {
        std::rotate(r.begin(), std::next(r.begin()), r.end());
        std::cout << "Test ring " << r << ": ";
        auto ts = ec::triangulate(r);
        normalize(ts);
        bool ok = ts == expected;
        std::cout << (ok ? "OK" : "Failed") << '\n';
        if (!ok) {
            std::cout << "Expected: " << expected << '\n' << "But got " << ts << '\n';
        }
        failed |= !ok;
    }

    return failed;
}

bool testTriangle(const ec::Ring& r, std::vector<ec::Triangle> expected)
{
    normalize(expected);
    std::cout << "Test triangulation. Expected: " << expected << '\n';
    std::cout << "Test ring " << r << ": ";
    auto ts = ec::triangulate(r);
    normalize(ts);
    bool ok = ts == expected;
    std::cout << (ok ? "OK" : "Failed") << '\n';
    if (!ok)
        std::cout << "Expected: " << expected << '\n' << "But got " << ts << '\n';

    return !ok;
}

bool testIntersects(ec::Point a, ec::Point b, ec::Point c, ec::Point d, bool expected) {
    std::cout << "Test intersects " << ec::Ring{a, b} << ", " << ec::Ring{c, d} << ": ";
    bool ok = ecd::intersects(a, b, c, d) == expected;
    if (ok) {
        std::cout << "Ok\n";
    } else {
        std::cout << "Failed\n";
    }
    return !ok;
}

bool testSelfIntersect(ec::Ring ring, const ec::Ring& expected)
{
    std::cout << "Test self-intersect: " << ring << ": ";
    ring = ecd::selfIntersect(std::move(ring));
    bool ok = (ring == expected);
    if (ok) {
        std::cout << "Ok\n";
    } else {
        std::cout << "Failed\n";
    }
    if (!ok)
        std::cout << "Expected: " << expected << '\n' << "But got " << ring << '\n';

    return !ok;
}

bool expectEqual(double a, double b) {
    constexpr double EPS = 0.0001;
    bool b1 = std::abs(a - b) < std::min(a, b) * EPS;
    if (!b1) {
        std::cout << "Test failed: " << a << "!= " << b << '\n';
    }

    return b1;
}

ec::Point rotate(ec::Point p, double a)
{
    auto cs = std::cos(a);
    auto sn = std::sin(a);
    return {p.x * cs - p.y * sn, p.x * sn + p.y * cs};
}

size_t testAngle() {
    std::cout << "Test angles: ";
    const std::vector<std::tuple<ec::Point, ec::Point, ec::Point, double>> CASES{
        {{1, 14}, {5, 10}, {9, 10}, M_PI * .75},
        {{1, 14}, {5, 10}, {7, 12}, M_PI * .5 },
        {{7, 8},  {5, 10}, {7, 12}, M_PI * 1.5},
        {{7, 12}, {5, 10}, {7, 8},  M_PI * 0.5},
    };
    size_t failedCount = 0;
    for (size_t i = 0; i < CASES.size(); ++i) {
        auto [a, b, c, expeced] = CASES[i];
        for (double alpha = -M_2_PI; alpha < M_PI * 2; alpha += 0.01 * M_PI) {
            double angle = ecd::angleRad(rotate(a, alpha), rotate(b, alpha), rotate(c, alpha));
            bool failed = !expectEqual(angle, expeced);
            failedCount += failed;
            if (failed)
                std::cout << "Test " << i << " angles failed\n";
        }
    }

    if (failedCount == 0) {
        std::cout << "Ok\n";
    } else {
        std::cout << "Failed\n";
    }

    return failedCount;
}

size_t testPointInTriangle() {
    std::cout << "Test point in tringle: ";
    std::vector<std::tuple<ec::Triangle, ec::Point, bool>> CASES {{
        {{{{0,1}, {1, 0}, {0, 0}}}, {0, 0}, false}
    }};
    size_t failedCount = 0;
    for (size_t i = 0; i < CASES.size(); ++i) {
        auto [t, p, expeced] = CASES[i];
        double in = ecd::pointInTriangle(t, p);
        bool failed = in != expeced;
        failedCount += failed;
        if (failed)
            std::cout << "Test point in triangle " << i << " angles failed\n";
    }

    if (failedCount == 0) {
        std::cout << "Ok\n";
    } else {
        std::cout << "Failed\n";
    }

    return failedCount;
}

int main()
{
    size_t failed = 0;
//    failed += testTriangleRotation({{{0, 0}, {0, 1}, {1, 0}}}, ecd::Direction::CWISE);
//    failed += testTriangleRotation({{{0, 0}, {1, 0}, {0, 1}}}, ecd::Direction::CCWISE);
//
//    failed += testRingRotation({{{0, 0}, {0, 1}, {1, 0}}}, ecd::Direction::CWISE);
//    failed += testRingRotation({{{0, 0}, {1, 0}, {0, 1}}}, ecd::Direction::CCWISE);
//
//    failed += testTriangle(
//        {{0, 0},
//         {1, 0},
//         {1, 1},
//         {0, 1}},
//        {{{{0, 0}, {0, 1}, {1, 0}}}, {{{0, 1}, {1, 0}, {1, 1}}}}
//        /*{{{{{0, 0}, {0, 1}, {1, 1}}}, {{{0, 0}, {1, 0}, {1, 1}}}}}*/);
//
//    failed += testTriangWithRotation(
//        {{{0, 0}, {1, 0}, {0, 1}}},
//        {{{{0, 0}, {1, 0}, {0, 1}}}});
//    failed += testTriangWithRotation(
//        {{{0, 0}, {4,2}, {2,4}, {2,2}}},
//        {{{{{0, 0}, {2, 2}, {4, 2}}},
//          {{{2, 2}, {2, 4}, {4, 2}}}}});
//
//    failed += testIntersects({0, 0}, {1, 1}, {0, 1}, {1, 0}, true);
//    failed += testIntersects({0, 0}, {0, 1}, {1, 1}, {1, 1}, false);
//    failed += testIntersects({0, 0}, {0, 1}, {0, 1}, {1, 1}, false);
//    failed += testIntersects({0, 0}, {2, 0}, {1, 1}, {1, 0}, false);
//
//    failed += testAngle();
//
//    failed += testSelfIntersect({{{0, 0}, {1, 0}, {0, 1}}}, {{{0, 1}, {1, 0}, {0, 0}}});
//    failed += testSelfIntersect({{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, {{{0.5, 0.5}, {0, 1}, {1, 1}, {0.5, 0.5}, {1, 0}, {0, 0}}});
//    failed += testSelfIntersect({{{0, -1}, {1, 0}, {0, 1}, {1, -1}}}, {{}});
//    testTriangle({{{0, -1}, {1, 0}, {0, 1}, {1, -1}}}, {{}});
    testTriangle({{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, {{}});
//    failed += testSelfIntersect({{{5, 2}, {10, 2}, {50, 50}, {91, 47}}}, {});
//    failed += testSelfIntersect({{0, 0}, {1, 0}, {0, 1}}, {{0, 1}, {0, 0}, {1, 0}});
//    failed += testPointInTriangle();
//    failed += testSelfIntersect({{{0, 8}, {7, 12}, {6, 0}, {10, 7}}}, {{}});

    if (failed == 0) {
        std::cout << "All test passed\n";
    } else {
        std::cout << failed << " tests failed\n";
    }

// {{276, 387}, {551, 75}, {662, 503}, {910, 296}, {823, 171}, {753, 135}, {727, 78}, {675, 88}, {675, 125}}

    return 0;
}
