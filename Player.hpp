#pragma once

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "Animation.hpp"
#include "Keyframe.hpp"

#include <glm/glm.hpp>

class Player {
public:
    Player(float pos_x, float pos_y);
    void move(bool hasCollision);
    glm::mat4 setDirection(int d);
    void removeDirection(int d);
    void setRootNode(SceneNode * n);
    void setJoints(JointNode * neck, JointNode * l, JointNode * r);

    float x;
    float y;
    float dx;
    float dy;
    float speed; // 1.0
    glm::mat4 currRot;
    
    double neckRotation;
    double leftThighRotation;
    double rightThighRotation;
    double leftThighDelta;
    double rightThighDelta;
    Animation moveAnimation;
    SceneNode * rootNode;
    JointNode * neckJoint;
    JointNode * leftThighJoint;
    JointNode * rightThighJoint;
};
