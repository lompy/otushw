#include "World.hpp"
#include "Painter.hpp"
#include <fstream>

static constexpr double timePerTick = 0.001;

World::World(const std::string& worldFilePath) {
    std::ifstream stream(worldFilePath);
    stream >> topLeft >> bottomRight;
    physics.setWorldBox(topLeft, bottomRight);

    Point point;
    Velocity velocity;
    Color color;
    double radius;
    bool collidable;

    while (stream.peek(), stream.good()) {
        stream >> point >> velocity >> color >> radius >> std::boolalpha >> collidable;
        balls.push_back(Ball(point, velocity, color, radius, collidable));
    }
}

void World::show(Painter& painter) const {
    painter.draw(topLeft, bottomRight, Color(1, 1, 1));

    for (const Ball& ball : balls) {
        ball.draw(painter);
    }
}

void World::update(double time) {
    time += restTime;
    const auto ticks = static_cast<size_t>(std::floor(time / timePerTick));
    restTime = time - double(ticks) * timePerTick;

    physics.update(balls, ticks);
}
