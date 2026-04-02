#pragma once

#include "VBO.h"



typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef int GLsizei;

class VAO
{
public:
    GLuint ID{0};
    VAO();
    ~VAO();


    VAO(const VAO&) = delete; // non construction-copyable
    VAO& operator=(const VAO&) = delete; // non copyable

    void Generate();
    void LinkIntVbo(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset);
    void LinkVBO(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset);
    void Bind();
    void Unbind();
    void Delete();
};

