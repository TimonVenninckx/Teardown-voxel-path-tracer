#pragma once

#include <glm/mat4x4.hpp>
#include "../texture/Texture2D.h"

class Shader;
class Framebuffer;

class ShadowMap2D
{
public:
    ShadowMap2D(unsigned int width,unsigned int height);

    void DrawMap(const glm::vec3& direction, Shader& shader,
        Framebuffer& frameBuffer);

    void BindTexture(int slot);
    void Unbind();
    unsigned int GetID()const { return texture.ID; }


    Texture2D texture;

    glm::mat4 lightSpaceMatrix{};
    float nearPlane{ -30.0f };
    float farPlane{ 10.f };
    float boxSize{ 30.f };
private:
};

