#include "Ball.hpp"

void Ball::setVelocity(const Velocity& val) {
    velocity = val;
}

Velocity Ball::getVelocity() const {
    return velocity;
}

void Ball::draw(Painter& painter) const {
    painter.draw(center, radius, color);
}

void Ball::setCenter(const Point& val) {
    center = val;
}

Point Ball::getCenter() const {
    return center;
}

double Ball::getRadius() const {
    return radius;
}

double Ball::getMass() const {
    return mass;
}

bool Ball::isCollidable() const {
    return collidable;
}
