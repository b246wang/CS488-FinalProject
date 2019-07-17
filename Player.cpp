#include "Player.hpp"

using namespace glm;
using namespace std;

Player::Player(vec3 p)
  : direction(0), // 0: south, 1: west, 2: north, 3: east
    previousDirection(0),
    rootNode(NULL),
    neckJoint(NULL),
    leftThighJoint(NULL),
    rightThighJoint(NULL),
    speed(0.0), // 0.0, 0.03, 0.05, 0.07, 0.09
    position(p),
    moveAnimation(Animation())
{
    map<string, double> jr1;
    jr1["neckJoint"] = 40.0;
    jr1["rightThighJoint"] = 15.0;
    jr1["leftThighJoint"] = -45.0;
    Keyframe frame1(0.1, jr1);
    moveAnimation.keyframes.push_back(frame1);

    map<string, double> jr2;
    jr2["neckJoint"] = 40.0;
    jr2["rightThighJoint"] = -60.0; // -45
    jr2["leftThighJoint"] = 60.0; // 15
    Keyframe frame2(0.2, jr2);
    moveAnimation.keyframes.push_back(frame2);

    map<string, double> jr3;
    jr3["neckJoint"] = -40.0; // 0
    jr3["rightThighJoint"] = 45.0; // 0
    jr3["leftThighJoint"] = -15.0; // 0
    Keyframe frame3(0.3, jr3);
    moveAnimation.keyframes.push_back(frame3);
    // cout << "moveAnimation 1st: " << moveAnimation.keyframes.front().timestamp << endl;
}

void Player::setDirection(int d) {
    previousDirection = direction;
    direction = d;
}

void Player::setRootNode(SceneNode * n) {
    rootNode = n;
}

void Player::setNeckJoint(JointNode * neck, JointNode * l, JointNode * r) {
    neckJoint = neck;
    leftThighJoint = l;
    rightThighJoint = r;
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
