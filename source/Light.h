#pragma once
#include <glm/vec3.hpp>

enum class LightType : uint{
    DirLight = 0,
    PointLight = 1,
    SpotLight = 2,
};


struct Light {
    glm::vec4 position{};
    glm::vec4 direction{};
    glm::vec4 color{ 1.f };
    LightType type{ LightType::DirLight };
    float innercutOff{ 0.91f };
    float outerCutOff{ 0.82f };
    float filler{};
};
