#pragma once

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "Animation.hpp"
#include "Keyframe.hpp"

#include <glm/glm.hpp>

class Player {
public:
    Player(glm::vec3 p);
    void move();
    void setDirection(int d);
    void setRootNode(SceneNode * n);
    void setJoints(JointNode * neck, JointNode * l, JointNode * r);

    int direction; // 0: south, 1: west, 2: north, 3: east
    int previousDirection;
    int speed;
    Animation moveAnimation;
    SceneNode * rootNode;
    JointNode * neckJoint;
    JointNode * leftThighJoint;
    JointNode * rightThighJoint;
    glm::vec3 position;
};
