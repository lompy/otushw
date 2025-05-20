#pragma once
#include "Color.hpp"
#include "Painter.hpp"
#include "Point.hpp"
#include "Velocity.hpp"

class Ball {
public:
    Ball() = default;
    inline Ball(Point center, Velocity velocity, Color color, double radius, bool collidable) :
        center {center}, velocity {velocity}, color{color},
        radius {radius}, collidable {collidable}, mass {radius*radius*radius} {};

    void setVelocity(const Velocity& velocity);
    Velocity getVelocity() const;
    void draw(Painter& painter) const;
    void setCenter(const Point& center);
    Point getCenter() const;
    double getRadius() const;
    double getMass() const;
    bool isCollidable() const;

private:
    Point center {};
    Velocity velocity {};
    Color color {};
    double radius {};
    bool collidable {};
    double mass {};
};
