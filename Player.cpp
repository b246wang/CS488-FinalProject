#include "Player.hpp"
#include "cs488-framework/MathUtils.hpp"

using namespace glm;
using namespace std;

static const float rotationDelta = 3.0;
static const float neckRotationDelta = 5.0;
static const float collision_square = 0.92f;
static const float init_speed = 0.07f;
static const float r_90 = degreesToRadians(90.0f);
static const float r_m_90 = degreesToRadians(-90.0f);
static const float r_180 = degreesToRadians(180.0f);
static const mat4 orig_rot = mat4(1.0f);
static const mat4 left_rot = mat4(
    cos(r_90), 0.0f, sin(r_90), 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    -sin(r_90), 0.0f, cos(r_90), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
static const mat4 right_rot = mat4(
    cos(r_m_90), 0.0f, sin(r_m_90), 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    -sin(r_m_90), 0.0f, cos(r_m_90), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
static const mat4 up_rot = mat4(
    cos(r_180), 0.0f, sin(r_180), 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    -sin(r_180), 0.0f, cos(r_180), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);

Player::Player(float pos_x, float pos_y)
  : x(pos_x),
    y(pos_y),
    dx(0.0f),
    dy(0.0f),
    currRot(mat4(1.0f)),
    neckRotation(0.0),
    leftThighRotation(0.0),
    rightThighRotation(0.0),
    leftThighDelta(-rotationDelta),
    rightThighDelta(rotationDelta),
    rootNode(NULL),
    neckJoint(NULL),
    leftThighJoint(NULL),
    rightThighJoint(NULL),
    speed(1.0f),
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

mat4 Player::setDirection(int d) {
    if (d == 0) {
        dy = init_speed;
        return orig_rot;
    } else if (d == 1) {
        dx = -init_speed;
        return left_rot;
    } else if (d == 2) {
        dy = -init_speed;
        return up_rot;
    } else if (d == 3) {
        dx = init_speed;
        return right_rot;
    }
    cout << "cannot find direction: " << d << endl;
    return orig_rot;
}

void Player::removeDirection(int d) {
    if (d == 0 && dy > 0.0f) {
        dy = 0.0f;
    } else if (d == 1 && dx < 0.0f) {
        dx = 0.0f;
    } else if (d == 2 && dy < 0.0f) {
        dy = 0.0f;
    } else if (d == 3 && dx > 0.0f) {
        dx = 0.0f;
    }
}

void Player::setRootNode(SceneNode * n) {
    rootNode = n;
}

void Player::setJoints(JointNode * neck, JointNode * l, JointNode * r) {
    neckJoint = neck;
    leftThighJoint = l;
    rightThighJoint = r;
}

void Player::move(bool hasCollision) {
    if (dx != 0.0f || dy != 0.0f) {
        // animate
        if (leftThighDelta < 0) {
            if (leftThighRotation > -15.0) {
                leftThighRotation += leftThighDelta;
                leftThighJoint->rotate('x', leftThighDelta);
            } else {
                leftThighDelta = rotationDelta;
            }
        } else {
            if (leftThighRotation < 15.0) {
                leftThighRotation += leftThighDelta;
                leftThighJoint->rotate('x', leftThighDelta);
            } else {
                leftThighDelta = -rotationDelta;
            }
        }
        
        if (rightThighDelta > 0) {
            if (rightThighRotation < 15.0) {
                rightThighRotation += rightThighDelta;
                rightThighJoint->rotate('x', rightThighDelta);
            } else {
                rightThighDelta = -rotationDelta;
            }
        } else {
            if (rightThighRotation > -15.0) {
                rightThighRotation += rightThighDelta;
                rightThighJoint->rotate('x', rightThighDelta);
            } else {
                rightThighDelta = rotationDelta;
            }
        }

        // move
        if (!hasCollision) {
            if (dx != 0.0f && dy != 0.0f) {
                rootNode->translate(vec3(dx * 0.7071 * speed, 0.0f, dy * 0.7071 * speed));
                x += dx * 0.7071 * speed;
                y += dy * 0.7071 * speed;
            } else {
                rootNode->translate(vec3(dx * speed, 0.0f, dy * speed));
                x += dx * speed;
                y += dy * speed;
            }
        }
    }
}
