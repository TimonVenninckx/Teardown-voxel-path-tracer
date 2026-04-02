#pragma once


#include "../buffers/VBO.h"
#include "../buffers/VAO.h"
#include "AABB.h"
#include <glm/fwd.hpp>

class Shader;

class Debug
{
public:
    Debug();
    ~Debug();

    void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float lifetime);
    void DrawBox(glm::vec3 center, glm::vec3 size, glm::vec3 color, float lifetime = 3.f);

    void DrawAABB(AABB aabb, glm::vec3 color, float lifetime = 3.f);

    void UpdateLifeTime(float deltaTime);

    void RebuildBufferIfNeccessary();

    void RenderDebugObjects(const glm::mat4& camera);

    GLsizei verticeCount{};
    Shader* debugShader;

    VAO vao;
private:
    VBO vbo;
};

