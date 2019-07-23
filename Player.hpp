#pragma once

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "Animation.hpp"
#include "Keyframe.hpp"

#include <glm/glm.hpp>

class Player {
public:
    Player(float pos_x, float pos_y);
    void setDead();
    void speedUp();
    void balloonUp();
    void powerUp();
    void move(bool hasCollision, bool shouldAnimate);
    glm::mat4 setDirection(int d);
    void removeDirection(int d);
    void setRootNode(SceneNode * n);
    void setJoints(JointNode * neck, JointNode * l, JointNode * r);

    bool damaged;
    float x;
    float y;
    float dx;
    float dy;
    float speed; // 1.0
    float power;
    bool dead;
    int balloonNumber;
    int health;
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
