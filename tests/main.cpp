#include <iostream>
#include <algorithm>
#include <map>
#include <tuple>
#include <cmath>
#include <iostream>

#include "ear_clip.h"

namespace ec = ear_clip;
namespace ecd = ear_clip::details;

inline std::ostream& operator<<(std::ostream& s, ecd::Direction direction) {
    static const std::map<ecd::Direction, std::string> DIRS = {
        {ecd::Direction::CWISE,  "Clockwise"},
        {ecd::Direction::CCWISE, "Counter clockwise"}
    };
    s << DIRS.at(direction);
    return s;
}

inline std::ostream& operator<<(std::ostream& s, const ec::Point& p) {
    s << "{" << p.x << ", " << p.y << "}";
    return s;
}

inline std::ostream& operator<<(std::ostream& s, const ec::Triangle& t) {
    s << "{" << t[0] << ", " << t[1] << ", " << t[2] << "}";
    return s;
}

inline std::ostream& operator<<(std::ostream& s, const ec::Ring& r) {
    s << "{";
    bool putComma = false;
    for (const auto& p: r) {
        if (putComma)
            s << ", ";
        else
            putComma = true;
        s << p;
    }
    s << "}";
    return s;
}

inline std::ostream& operator<<(std::ostream& s, const std::vector<ec::Triangle>& r) {
    s << "{";
    bool putComma = false;
    for (const auto& p: r) {
        if (putComma) {
            s << ", ";
        } else {
            putComma = true;
        }
        s << p;
    }
    s << "}";

    return s;
}

bool expectEqual(double a, double b) {
    constexpr double EPS = 0.0001;
    bool equal = std::abs(a - b) < std::min(a, b) * EPS;
    if (!equal)
        std::cout << "Test failed: " << a << "!= " << b << '\n';

    return equal;
}

bool expectEqual(ec::Point a, ec::Point b) {
    bool equal = expectEqual(a.x, b.x) && expectEqual(a.y, b.y);
    if (!equal)
        std::cout << "Test failed: " << a << "!= " << b << '\n';

    return equal;
}

bool expectEqual(ec::Triangle a, ec::Triangle b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    bool equal = true;
    for (auto i = 0; i < 3; ++i)
        equal &= expectEqual(a[i], b[i]);
    if (!equal)
        std::cout << "Test failed: " << a << "!= " << b << '\n';

    return equal;
}

bool expectEqual(std::vector<ec::Triangle> a, std::vector<ec::Triangle> b) {
    if (a.size() != b.size()) {
        std::cout << "Test failed: size " << a.size() << "!= " << b.size() << '\n';
        return false;
    }

    for (auto& t: a) std::sort(t.begin(), t.end());
    for (auto& t: b) std::sort(t.begin(), t.end());
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    bool equal = true;
    for (auto i = 0; i < a.size(); ++i)
        equal &= expectEqual(a[i], b[i]);
    if (!equal)
        std::cout << "Test failed: " << a << "!= " << b << '\n';

    return equal;
}

bool testRingRotation(ec::Ring r, ecd::Direction dir_tar)
{
    std::cout << "Test ring rotation. Expected: " << dir_tar << '\n';
    bool failed = false;
    for (size_t i = 0; i < r.size(); ++i) {
        std::rotate(r.begin(), std::next(r.begin()), r.end());
        std::cout << "Test ring " << r << ": ";
        auto dir = ecd::ringDirection(r);
        bool ok = dir == dir_tar;
        std::cout << (ok ? "OK" : "Failed") << '\n';
        failed |= !ok;
    }
    return failed;
}

bool testTriangulate(const ec::Ring& r, const std::vector<ec::Triangle>& expected)
{
    std::cout << "Test triangulation. Expected: " << expected << '\n';
    std::cout << "Test ring " << r << ": ";
    auto ts = ec::triangulate(r);
    bool ok = expectEqual(ts, expected);
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


ec::Point rotate(ec::Point p, double a)
{
    auto cs = std::cos(a);
    auto sn = std::sin(a);
    return {p.x * cs - p.y * sn, p.x * sn + p.y * cs};
}

size_t testAngle() {
    std::cout << "Test angles: ";
    const std::vector<std::tuple<ec::Point, ec::Point, ec::Point, double>> CASES{
        {{-1, 0}, {0, 0}, {1, 0}, M_PI},
        {{1, 14}, {5, 10}, {9, 10}, M_PI * .75},
        {{1, 14}, {5, 10}, {7, 12}, M_PI * .5 },
        {{7, 8},  {5, 10}, {7, 12}, M_PI * 1.5},
        {{7, 12}, {5, 10}, {7, 8},  M_PI * .5},
    };
    size_t failedCount = 0;
    for (size_t i = 0; i < CASES.size(); ++i) {
        auto [a, b, c, expeced] = CASES[i];
        for (double alpha = -M_2_PI; alpha < M_PI * 2; alpha += 0.01 * M_PI) {
            double angle = ecd::angleRad(rotate(a, alpha), rotate(b, alpha), rotate(c, alpha));
            bool failed = !expectEqual(angle, expeced);
            failedCount += failed;
            if (failed) {
                std::cout << "Test " << i << " alpha=" << alpha << " failed\n";
                break;
            }
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
    std::cout << "Test point in triangle: ";
    std::vector<std::tuple<ec::Triangle, ec::Point, bool>> CASES {{
        {{{{0,1}, {1, 0}, {0, 0}}}, {0, 0}, false},
        {{{{0,1}, {0, 0}, {1, 0}}}, {0, 0}, false},
        {{{{0,1}, {1, 0}, {0, 0}}}, {100, 100}, false},
        {{{{0,1}, {0, 0}, {0, 1}}}, {100, 100}, false},
        {{{{0,0}, {1, 0}, {0.5, 10}}}, {0.5, 0.5}, true},
        {{{{0,0}, {0.5, 10}, {1, 0}}}, {0.5, 0.5}, true},
    }};
    size_t failedCount = 0;
    for (size_t i = 0; i < CASES.size(); ++i) {
        auto [t, p, expeced] = CASES[i];
        double in = ecd::pointInTriangle(t, p);
        bool failed = in != expeced;
        failedCount += failed;
        if (failed) {
            std::cout << "Test point in triangle test case# " << i << ": Failed\n";
        }
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

    failed += testRingRotation({{{0, 0}, {0, 1}, {1, 0}}}, ecd::Direction::CWISE);
    failed += testRingRotation({{{0, 0}, {1, 0}, {0, 1}}}, ecd::Direction::CCWISE);

    failed += testIntersects({0, 0}, {1, 1}, {0, 1}, {1, 0}, true);
    failed += testIntersects({0, 0}, {0, 1}, {1, 1}, {1, 1}, false);
    failed += testIntersects({0, 0}, {0, 1}, {0, 1}, {1, 1}, false);
    failed += testIntersects({0, 0}, {2, 0}, {1, 1}, {1, 0}, false);

    failed += testAngle();

    const ec::Ring simplestRing = {{{0, 0}, {1, 0}, {0, 1}}};
    const ec::Ring repeatPoint = {{{0, 0}, {1, 0}, {1, 0}, {0, 1}}};
    const ec::Ring ring8 = {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}};
    const ec::Ring ringM = {{{1, 1}, {3, 3}, {5, 1}, {5, 2}, {1, 2}}};

    failed += testSelfIntersect(simplestRing, {{{1, 0}, {0, 1}, {0, 0}}});
    // repeat point
    failed += testSelfIntersect(repeatPoint, {{{1, 0}, {0, 1}, {0, 0}}});
    // 8-test
    failed += testSelfIntersect(ring8, {{1, 0}, {0.5, 0.5}, {1, 1}, {0, 1}, {0.5, 0.5}, {0, 0}});
    // M-test
    failed += testSelfIntersect(ringM, {{2, 2}, {4, 2}, {5, 1}, {5, 2}, {4, 2}, {3, 3}, {2, 2}, {1, 2}, {1, 1}});

// We are here
//    failed += testSelfIntersect({{{0, -1}, {1, 0}, {0, 1}, {1, -1}}}, {{}});
//    testTriangulate({{{0, -1}, {1, 0}, {0, 1}, {1, -1}}}, {{}});
//    testTriangulate({{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, {{}});
//    failed += testSelfIntersect({{{5, 2}, {10, 2}, {50, 50}, {91, 47}}}, {});
//    failed += testSelfIntersect({{0, 0}, {1, 0}, {0, 1}}, {{0, 1}, {0, 0}, {1, 0}});
    failed += testPointInTriangle();
//    failed += testSelfIntersect({{{0, 8}, {7, 12}, {6, 0}, {10, 7}}}, {{}});

    if (failed == 0) {
        std::cout << "All test passed\n";
    } else {
        std::cout << failed << " tests failed\n";
    }

// {{276, 387}, {551, 75}, {662, 503}, {910, 296}, {823, 171}, {753, 135}, {727, 78}, {675, 88}, {675, 125}}

// triangulate test: 0 1 1 0 1 2 2 1

    return 0;
}
