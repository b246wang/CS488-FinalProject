#pragma once

#include "SceneNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>

class Player {
public:
    Player(glm::vec3 p);
    void move();
    void setDirection(int d);
    void setRootNode(SceneNode * n);
    void setNeckJoint(JointNode * n);

    int direction; // 0: south, 1: west, 2: north, 3: east
    int previousDirection;
    int speed;
    SceneNode * rootNode;
    JointNode * neckJoint;
    glm::vec3 position;
};
