#pragma once

#include "SceneNode.hpp"

#include <glm/glm.hpp>

class Player {
public:
    Player(glm::vec3 p);
    void move();
    void setDirection(int d);
    void setRootNode(SceneNode * n);

    int direction; // 0: south, 1: west, 2: north, 3: east
    int previousDirection;
    int speed;
    SceneNode * rootNode;
    glm::vec3 position;
};
