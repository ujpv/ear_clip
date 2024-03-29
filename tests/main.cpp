#include <iostream>
#include <algorithm>
#include <map>
#include <tuple>
#include <cmath>
#include <iostream>

#include "ear_clip.h"

namespace ec = ear_clip;
namespace ecd = ear_clip::details;

inline std::ostream &operator<<(std::ostream &s, ecd::VertexOrder direction) {
  static const std::map<ecd::VertexOrder, std::string> DIRS = {
      {ecd::VertexOrder::CLOCKWISE, "Clockwise"},
      {ecd::VertexOrder::C_CLOCKWISE, "Counter clockwise"},
      {ecd::VertexOrder::NO_AREA, "No area"},
  };
  s << DIRS.at(direction);
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const ec::Point &p) {
  s << "{" << p.x << ", " << p.y << "}";
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const ec::Triangle &t) {
  s << "{{" << t[0] << ", " << t[1] << ", " << t[2] << "}}";
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const ec::Ring &r) {
  s << "{";
  bool putComma = false;
  for (const auto &p : r) {
    if (putComma)
      s << ", ";
    else
      putComma = true;
    s << p;
  }
  s << "}";
  return s;
}

inline std::ostream &operator<<(std::ostream &s, const std::vector<ec::Triangle> &r) {
  s << "{";
  bool putComma = false;
  for (const auto &p : r) {
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
  constexpr double EPS = 0.00001;
  bool equal = std::abs(a - b) <= std::max(std::abs(a), std::abs(b)) * EPS;
  if (!equal)
    std::cout << "Test failed: " << a << " != " << b << '\n';

  return equal;
}

bool expectEqual(ec::Point a, ec::Point b) {
  bool equal = expectEqual(a.x, b.x) && expectEqual(a.y, b.y);
  if (!equal)
    std::cout << "Test failed: " << a << " != " << b << '\n';

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
    std::cout << "Test failed: " << a << " != " << b << '\n';
    return false;
  }

  for (auto &t : a) std::sort(t.begin(), t.end());
  for (auto &t : b) std::sort(t.begin(), t.end());
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());

  bool equal = true;
  for (auto i = 0; i < a.size(); ++i)
    equal &= expectEqual(a[i], b[i]);
  if (!equal)
    std::cout << "Test failed: " << a << " != " << b << '\n';

  return equal;
}

bool expectEqual(const ec::Ring &a, const ec::Ring &b) {
  if (a.size() != b.size()) {
    std::cout << "Test size failed: " << a.size() << " != " << b.size() << '\n';
    return false;
  }

  bool equal = true;
  for (auto ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi) {
    equal &= expectEqual(*ai, *bi);
  }

  if (!equal)
    std::cout << "Test failed: " << a << " != " << b << '\n';

  return equal;
}

bool testRingRotation(ec::Ring r, ecd::VertexOrder dir_tar) {
  std::cout << "Test ring rotation. VertexOrder: " << dir_tar << '\n';
  bool failed = false;
  for (size_t i = 0; i < r.size(); ++i) {
    std::rotate(r.begin(), std::next(r.begin()), r.end());
    std::cout << "Test ring " << r << ": ";
    auto dir = ecd::vertexOrder(r);
    bool ok = dir == dir_tar;
    std::cout << (ok ? "OK" : "Failed") << '\n';
    failed |= !ok;
  }
  return failed;
}

bool testTriangulate(const ec::Ring &r,
                     const std::vector<ec::Triangle> &expected, const std::string &name) {
  std::cout << "Test triangulation. " << name << ": ";
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

bool testNormalize(ec::Ring ring, const ec::Ring &expected) {
  std::cout << "Test normalize: " << ring << ": ";
  ring = ecd::normalizeRing(std::move(ring));
  bool ok = expectEqual(ring, expected);
  if (ok) {
    std::cout << "Ok\n";
  } else {
    std::cout << "Failed\n";
  }
  if (!ok)
    std::cout << "Expected: " << expected << '\n' << "But got " << ring << '\n';

  return !ok;
}

ec::Point rotate(ec::Point p, double a) {
  auto cs = std::cos(a);
  auto sn = std::sin(a);
  return {p.x * cs - p.y * sn, p.x * sn + p.y * cs};
}

size_t testAngle() {
  std::cout << "Test angles: ";
  const std::vector<std::tuple<ec::Point, ec::Point, ec::Point, double>> CASES{
      {{-1, 0}, {0, 0}, {1, 0}, M_PI},
      {{1, 14}, {5, 10}, {9, 10}, M_PI * .75},
      {{1, 14}, {5, 10}, {7, 12}, M_PI * .5},
      {{7, 8}, {5, 10}, {7, 12}, M_PI * 1.5},
      {{7, 12}, {5, 10}, {7, 8}, M_PI * .5},
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
  std::vector<std::tuple<ec::Triangle, ec::Point, bool>> CASES{{
                                                                   {{{{0, 1}, {1, 0}, {0, 0}}}, {0, 0}, false},
                                                                   {{{{0, 1}, {0, 0}, {1, 0}}}, {0, 0}, false},
                                                                   {{{{0, 1}, {1, 0}, {0, 0}}}, {100, 100}, false},
                                                                   {{{{0, 1}, {0, 0}, {0, 1}}}, {100, 100}, false},
                                                                   {{{{0, 0}, {1, 0}, {0.5, 10}}}, {0.5, 0.5}, true},
                                                                   {{{{0, 0}, {0.5, 10}, {1, 0}}}, {0.5, 0.5}, true},
                                                                   {{{{0, 0}, {1, 0}, {0, 1}}}, {0.5, 0}, true},
                                                                   {{{{0, 0}, {0, 1}, {1, 0}}}, {0.5, 0}, true},
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

int main() {
  size_t failed = 0;
  ec::enableTrace(false);

  failed += testRingRotation({{{0, 0}, {0, 1}, {1, 0}}}, ecd::VertexOrder::CLOCKWISE);
  failed += testRingRotation({{{0, 0}, {1, 0}, {0, 1}}}, ecd::VertexOrder::C_CLOCKWISE);
  failed += testRingRotation({{{0, 0}, {1, 0}, {2, 0}}}, ecd::VertexOrder::NO_AREA);
  failed += testRingRotation({{{0, 0}, {1, 0}, {0, 0}}}, ecd::VertexOrder::NO_AREA);

  failed += testIntersects({0, 0}, {1, 1}, {0, 1}, {1, 0}, true);
  failed += testIntersects({0, 0}, {0, 1}, {1, 1}, {1, 1}, false);
  failed += testIntersects({0, 0}, {0, 1}, {0, 1}, {1, 1}, false);
  failed += testIntersects({0, 0}, {2, 0}, {1, 1}, {1, 0}, false);

  failed += testPointInTriangle();

  failed += testAngle();

  const ec::Ring simplestRing = {{{0, 0}, {1, 0}, {0, 1}}};
  const ec::Ring repeatPoint = {{{0, 0}, {1, 0}, {1, 0}, {0, 1}}};
  const ec::Ring square = {{{-1, -1}, {1, -1}, {1, 1}, {-1, 1}}};
  const ec::Ring selfTouch = {{{0, 2}, {2, 0}, {4, 2}, {3, 4}, {2, 2}, {1, 4}}};
  const ec::Ring selfTouch2 = {{{0, 8}, {2, 0}, {4, 8}, {3, 6}, {2, 8}, {1, 6}}};
  const ec::Ring selfTouchSaw = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 0}, {2, 1}, {3, 0}, {3, 1}, {4, 0}}};
  const ec::Ring ring8 = {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}};
  const ec::Ring ring8Complex =
      {{276, 387}, {551, 75}, {662, 503}, {910, 296}, {823, 171}, {753, 135}, {727, 78}, {675, 88}, {675, 125}};
  const ec::Ring ring8Complex2 =
      {{204, 532}, {115, 454}, {98, 357}, {161, 283}, {367, 559}, {442, 270}, {582, 533}, {666, 241}, {826, 496},
       {900, 234}, {960, 235}, {1013, 270}, {1040, 316}, {1044, 398}, {1034, 461}, {1006, 479}, {765, 295}, {710, 499},
       {547, 345}, {458, 517}, {309, 362}};
  const ec::Ring ring8_45 = {{{0, 1}, {1, 0}, {1, 2}, {2, 1}}};
  const ec::Ring ringM = {{{1, 1}, {3, 3}, {5, 1}, {5, 2}, {1, 2}}};
  const ec::Ring ringInf =
      {{{56, 485}, {175, 80}, {290, 544}, {444, 85}, {471, 581}, {628, 87}, {680, 600}, {822, 100}, {863, 732},
        {891, 295}}};
  const ec::Ring ringCross = {{-1, -2}, {1, -2}, {0, 0},
                              {-2, 1}, {-1, 2}, {0, 0},
                              {2, 1}, {1, 2}, {0, 0}};

  const ec::Ring zeroAreaLoop = {{{0, 0}, {1, 0}, {0, 0}}};
  const ec::Ring zeroAreaLoop2 = {{{0, 0}, {1, 0}, {2, 0}, {1, 0}}};
  const ec::Ring zeroAreaLoop3 = {{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {2, 0}, {1, 0}}};
  const ec::Ring zeroAreaLoopStart = {{{0, -1}, {0, 0},
                                       {1, 1}, {0, 0},
                                       {-1, 1}, {0, 0}}};
  const ec::Ring zeroAreaTriangleBag = {{118, 553}, {554, 151}, {552, 623},
                                        {308, 113}, {428, 741}, {252, 761},
                                        {154, 723}, {116, 689}};

  failed += testNormalize({}, {});
  failed += testNormalize(simplestRing, {{{1, 0}, {0, 1}, {0, 0}}});
  failed += testNormalize(repeatPoint, {{{1, 0}, {0, 1}, {0, 0}}});
  failed += testNormalize(square, {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}});
  failed += testNormalize(selfTouch, {{2, 0}, {4, 2}, {3, 4}, {2, 2}, {1, 4}, {0, 2}});
  failed += testNormalize(selfTouch2, {{2, 0}, {4, 8}, {3, 6}, {2, 8}, {1, 6}, {0, 8}});
  failed += testNormalize(selfTouchSaw, {{4, 0}, {3, 1}, {3, 0}, {2, 1}, {2, 0}, {1, 1}, {1, 0}, {0, 1}, {0, 0}});
  failed += testNormalize(ring8, {{1, 0}, {0.5, 0.5}, {1, 1}, {0, 1}, {0.5, 0.5}, {0, 0}});
  failed += testNormalize(ring8_45, {{1, 0}, {1, 1}, {2, 1}, {1, 2}, {1, 1}, {0, 1}});
  failed += testNormalize(ringM, {{2, 2}, {4, 2}, {5, 1}, {5, 2}, {4, 2}, {3, 3}, {2, 2}, {1, 2}, {1, 1}});
  failed += testNormalize(ringCross, {{0, 0}, {-1, -2}, {1, -2}, {0, 0}, {2, 1}, {1, 2}, {0, 0}, {-1, 2}, {-2, 1}});
  failed += testNormalize(ringInf,
                          {{175, 80}, {263.666, 437.747}, {330.772, 422.477}, {444, 85}, {460.761, 392.899},
                           {536.24, 375.724}, {628, 87}, {654.538, 348.806}, {758.027, 325.257}, {822, 100},
                           {835.47, 307.636}, {891, 295}, {863, 732}, {835.47, 307.636}, {758.027, 325.257}, {680, 600},
                           {654.538, 348.806}, {536.24, 375.724}, {471, 581}, {460.761, 392.899}, {330.772, 422.477},
                           {290, 544}, {263.666, 437.747}, {56, 485}});

  failed += testNormalize(zeroAreaLoop, {{1, 0}, {0, 0}});
  failed += testNormalize(zeroAreaLoop2, {{1, 0}, {2, 0}, {1, 0}, {0, 0}});
  failed += testNormalize(zeroAreaLoop3, {{1, 0}, {2, 0}, {3, 0}, {2, 0}, {1, 0}, {0, 0}});
  failed += testNormalize(zeroAreaLoopStart,
                          {{0, 0}, {0, -1}, {0, 0}, {1, 1}, {0, 0}, {-1, 1}});

  failed += testTriangulate(zeroAreaLoop, {}, "Empty loop");
  failed += testTriangulate(zeroAreaLoop2, {}, "Empty loop2");
  failed += testTriangulate(zeroAreaLoop3, {}, "Empty loop3");
  failed += testTriangulate(zeroAreaLoopStart, {}, "Empty loop star");

  failed += testTriangulate({}, {}, "Empty");
  failed += testTriangulate(
      simplestRing,
      {{{{1, 0}, {0, 1}, {0, 0}}}},
      "Simple");
  failed += testTriangulate(
      square,
      {{{{1, -1}, {1, 1}, {-1, 1}}}, {{{1, -1}, {-1, 1}, {-1, -1}}}},
      "Square");
  failed += testTriangulate(
      selfTouch,
      {{{{2, 0}, {4, 2}, {3, 4}}}, {{{2, 0}, {3, 4}, {2, 2}}}, {{{2, 0}, {2, 2}, {1, 4}}}, {{{2, 0}, {1, 4}, {0, 2}}}},
      "Self-touch");
  failed += testTriangulate(
      selfTouch2,
      {{{{2, 0}, {4, 8}, {3, 6}}}, {{{2, 0}, {3, 6}, {2, 8}}}, {{{2, 0}, {2, 8}, {1, 6}}}, {{{2, 0}, {1, 6}, {0, 8}}}},
      "Self-touch 2");
  failed += testTriangulate(
      selfTouchSaw,
      {{{{4, 0}, {3, 1}, {3, 0}}}, {{{3, 0}, {2, 1}, {2, 0}}}, {{{2, 0}, {1, 1}, {1, 0}}}, {{{1, 0}, {0, 1}, {0, 0}}}},
      "Self-touch a saw");
  failed += testTriangulate(
      repeatPoint,
      {{{{1, 0}, {0, 1}, {0, 0}}}},
      "Repeating point");
  failed += testTriangulate(
      ringM,
      {{{{4, 2}, {5, 1}, {5, 2}}}, {{{4, 2}, {3, 3}, {2, 2}}}, {{{2, 2}, {1, 2}, {1, 1}}}},
      "M-ring");
  failed += testTriangulate(
      ring8,
      {{{{0.5, 0.5}, {1, 1}, {0, 1}}}, {{{0.5, 0.5}, {0, 0}, {1, 0}}}},
      "8-ring");
  failed += testTriangulate(
      ring8_45,
      {{{{1, 1}, {2, 1}, {1, 2}}}, {{{1, 1}, {0, 1}, {1, 0}}}},
      "8-ring x 45");
  failed += testTriangulate(
      ring8Complex,
      {{{{675, 125}, {675, 88}, {727, 78}}}, {{{675, 125}, {727, 78}, {753, 135}}},
       {{{675, 125}, {753, 135}, {823, 171}}}, {{{675, 125}, {823, 171}, {910, 296}}},
       {{{675, 125}, {910, 296}, {662, 503}}}, {{{675, 125}, {662, 503}, {580.124, 187.299}}},
       {{{580.124, 187.299}, {276, 387}, {551, 75}}}},
      "8-ring complex");
  failed += testTriangulate(
      ring8Complex2,
      {{{{268.683, 427.275}, {309, 362}, {394.927, 451.387}}}, {{{394.927, 451.387}, {442, 270}, {514.923, 406.991}}},
       {{{514.923, 406.991}, {547, 345}, {617.045, 411.177}}}, {{{617.045, 411.177}, {666, 241}, {745.429, 367.59}}},
       {{{745.429, 367.59}, {765, 295}, {861.88, 368.966}}}, {{{861.88, 368.966}, {900, 234}, {960, 235}}},
       {{{861.88, 368.966}, {960, 235}, {1013, 270}}}, {{{861.88, 368.966}, {1013, 270}, {1040, 316}}},
       {{{861.88, 368.966}, {1040, 316}, {1044, 398}}}, {{{861.88, 368.966}, {1044, 398}, {1034, 461}}},
       {{{861.88, 368.966}, {1034, 461}, {1006, 479}}}, {{{861.88, 368.966}, {826, 496}, {745.429, 367.59}}},
       {{{745.429, 367.59}, {710, 499}, {617.045, 411.177}}}, {{{617.045, 411.177}, {582, 533}, {514.923, 406.991}}},
       {{{514.923, 406.991}, {458, 517}, {394.927, 451.387}}}, {{{394.927, 451.387}, {367, 559}, {268.683, 427.275}}},
       {{{268.683, 427.275}, {204, 532}, {115, 454}}}, {{{268.683, 427.275}, {115, 454}, {98, 357}}},
       {{{268.683, 427.275}, {98, 357}, {161, 283}}}},
      "8-ring complex2");
  failed += testTriangulate(
      ringCross,
      {{{{0, 0}, {-1, -2}, {1, -2}}}, {{{0, 0}, {2, 1}, {1, 2}}}, {{{0, 0}, {-1, 2}, {-2, 1}}}},
      "Ring cross");
  failed += testTriangulate(
      ringInf,
      {{{{330.772, 422.477}, {444, 85}, {460.761, 392.899}}}, {{{536.24, 375.724}, {628, 87}, {654.538, 348.806}}},
       {{{536.24, 375.724}, {654.538, 348.806}, {758.027, 325.257}}},
       {{{758.027, 325.257}, {822, 100}, {835.47, 307.636}}}, {{{835.47, 307.636}, {891, 295}, {863, 732}}},
       {{{758.027, 325.257}, {680, 600}, {654.538, 348.806}}}, {{{536.24, 375.724}, {471, 581}, {460.761, 392.899}}},
       {{{536.24, 375.724}, {460.761, 392.899}, {330.772, 422.477}}},
       {{{330.772, 422.477}, {290, 544}, {263.666, 437.747}}}, {{{330.772, 422.477}, {263.666, 437.747}, {56, 485}}},
       {{{330.772, 422.477}, {56, 485}, {175, 80}}}, {{{263.666, 437.747}, {330.772, 422.477}, {460.761, 392.899}}},
       {{{654.538, 348.806}, {536.24, 375.724}, {330.772, 422.477}}},
       {{{654.538, 348.806}, {330.772, 422.477}, {175, 80}}}, {{{758.027, 325.257}, {654.538, 348.806}, {175, 80}}}},
      "Inf");

  failed += testTriangulate(zeroAreaTriangleBag, {
                                {{{{351.022, 338.149}, {308, 113}, {395.915, 296.757}}}, {{{395.915, 296.757}, {554, 151}, {552, 623}}},
                                 {{{351.022, 338.149}, {428, 741}, {252, 761}}}, {{{351.022, 338.149}, {252, 761}, {154, 723}}},
                                 {{{351.022, 338.149}, {154, 723}, {116, 689}}}, {{{351.022, 338.149}, {116, 689}, {118, 553}}}}},
                            "Zero area triangle");

  if (failed == 0) {
    std::cout << "All test passed\n";
  } else {
    std::cout << failed << " tests failed\n";
  }

  return 0;
}
