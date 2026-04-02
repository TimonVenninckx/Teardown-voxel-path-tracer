#include "VAO.h"
#include "glad/glad.h"


VAO::VAO()
{
}

VAO::~VAO()
{
    Delete();
}


void VAO::Generate() {
    if (ID == 0) {
        glGenVertexArrays(1, &ID);
    }
}

void VAO::LinkIntVbo(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset)
{
    Generate();
    VBO.Bind();
    glVertexAttribIPointer(layout, numComponents, type, stride, offset);
    glEnableVertexAttribArray(layout);
}

void VAO::LinkVBO(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset)
{
    Generate();
    VBO.Bind();
    glVertexAttribPointer(layout,numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
}

void VAO::Bind()
{
    Generate();
    glBindVertexArray(ID);
}

void VAO::Unbind()
{
    glBindVertexArray(0);
}
void VAO::Delete()
{
    if (ID != 0) {
        glDeleteVertexArrays(1, &ID);
        Unbind();
    }
}
