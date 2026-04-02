#pragma once



#include <glad/glad.h>
#include <vector>

class EBO
{
public:
    GLuint ID;
    EBO();

    inline void Generate();
    void BufferData(std::vector<GLuint>& indices);

    void Bind();
    void Unbind();
    void Delete();
};