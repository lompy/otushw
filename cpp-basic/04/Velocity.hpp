#pragma once
#include "Point.hpp"
#include <cmath>

class Velocity {
public:
    inline Velocity() = default;

    inline Velocity(double abs, double angle) {
        const double x = std::cos(angle);
        const double y = std::sin(angle);
        vec = Point{x, y} * abs;
    }

    inline Velocity(const Point& vector) {
        setVector(vector);
    }

    inline void setVector(const Point& vector) {
        vec = vector;
    }

    inline Point vector() const {
        return vec;
    }

private:
    Point vec;
};

inline std::istream& operator>>(std::istream& in, Velocity& velocity) {
    Point p;
    in >> p;
    velocity.setVector(p);

    return in;
}
