#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include <vector>

struct Vertex {
    glm::vec3 position{};
    glm::vec2 texUV{};
    glm::vec3 normal{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};

struct DebugVertex {
    glm::vec3 position{};
    glm::vec3 color{};
};


struct EntityVertex {
    glm::vec3 position{};
    glm::vec3 texUV{};
};

typedef unsigned int GLuint;

class VBO
{
public:
    GLuint ID{ 0 };
    VBO();
    ~VBO();

    VBO(const VBO&) = delete; // non construction-copyable
    VBO& operator=(const VBO&) = delete; // non copyable


    inline void Generate();

    template <typename T>
    void BufferData(std::vector<T>& vertices)
    {
        ActuallyBufferData(vertices.data(), vertices.size() * sizeof(T));
    }

    void Bind();
    void Unbind();
    void Delete();
private:
    void ActuallyBufferData(void* data, std::size_t dataSizeInBytes);
};

