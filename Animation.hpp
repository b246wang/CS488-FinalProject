#pragma once

#include "Keyframe.hpp"

#include <vector>

class Animation {
public:
    Animation();
    Animation(std::vector<Keyframe> kfs);
    std::vector<Keyframe> keyframes;
};
