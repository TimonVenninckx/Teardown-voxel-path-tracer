#pragma once

#include "engine/Common.h" 
#include <glm/vec3.hpp>


struct Sphere {
    glm::vec3 pos;
    float radius;
    uint materialID;
    float filler2[3];
};