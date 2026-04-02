#include "EBO.h"


EBO::EBO()
{
}

void EBO::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

inline void EBO::Generate() {
    if (ID == 0) {
        glGenBuffers(1, &ID);
    }
}

void EBO::BufferData(std::vector<GLuint>& indices) {
    Generate();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}


void EBO::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void EBO::Delete()
{
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        Unbind();
    }
}
