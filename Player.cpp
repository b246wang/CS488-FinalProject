#include "Player.hpp"

using namespace glm;

Player::Player(vec3 p)
  : direction(0), // 0: south, 1: west, 2: north, 3: east
    previousDirection(0),
    rootNode(NULL),
    speed(0.0), // 0.0, 0.03, 0.05, 0.07, 0.09
    position(p)
{}

void Player::setDirection(int d) {
    previousDirection = direction;
    direction = d;
}

void Player::setRootNode(SceneNode * n) {
    rootNode = n;
}

void Player::move() {
    if (direction == 1) {
        rootNode->translate(vec3(-0.07f + speed, 0.0, 0.0f));
    } else if (direction == 3) {
        rootNode->translate(vec3(0.07f + speed, 0.0, 0.0f));
    } else if (direction == 2) {
        rootNode->translate(vec3(0.0f, 0.0, -0.07f + speed));
    } else if (direction == 0) {
        rootNode->translate(vec3(0.0f, 0.0, 0.07f + speed));
    }
}
