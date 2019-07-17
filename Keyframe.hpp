#pragma once

#include <map>
#include <string>

class Keyframe {
public:
    Keyframe(double t, std::map<std::string, double> jr);
    double timestamp;
    std::map<std::string, double> jointRotations;
};
