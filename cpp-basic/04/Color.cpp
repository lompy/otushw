#include "Color.hpp"
#include <istream>

Color::Color() = default;

Color::Color(double red, double green, double blue)
    : r{red}, g{green}, b{blue} {}

double Color::red() const {
    return r;
}

double Color::green() const {
    return g;
}

double Color::blue() const {
    return b;
}

std::istream& operator>>(std::istream& in, Color& color) {
    in >> color.r >> color.g >> color.b;

    return in;
}
