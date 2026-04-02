#pragma once

#include "engine/Common.h"

enum class MaterialType : uint {
    Diffuse = 0,
    Metal = 1,
    Glass = 2,
    Emit = 3
};

struct Material {
    glm::vec3 color{};
    MaterialType type{};
    float emit{};
    float flux{};
    float ior{};
    float filler[1]{};
};
