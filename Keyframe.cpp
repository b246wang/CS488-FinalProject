#include "Keyframe.hpp"

Keyframe::Keyframe(double t, std::map<std::string, double> jr) : 
    timestamp(t), jointRotations(jr)
{}
