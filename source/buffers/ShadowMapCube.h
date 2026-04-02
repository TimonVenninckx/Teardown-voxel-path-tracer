#pragma once

#include <glm/glm.hpp>
#include "../engine/Shader.h"

#include "../texture/TextureCubemap.h"
#include "Framebuffer.h"

class ShadowMapCube
{
public:
    ShadowMapCube(unsigned int width, unsigned int height);

    void DrawMap(const glm::vec3& lightPosition, Shader& shader, 
        Framebuffer& frameBuffer);

    void Bind(int slot);
    void Unbind();

    unsigned int GetID() { return cubemap.ID; }

    float nearPlane{ 0.1f };
    float farPlane{ 25.f };

    TextureCubemap cubemap;
private:

    glm::mat4 shadowTransforms[6]{};
};

