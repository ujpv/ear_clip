#pragma once
#include <iostream>

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
