#include "ScreenQuad.h"

#include <glad/glad.h>


struct ScreenVertex {
    glm::vec2 pos;
    glm::vec2 texUV;
};

ScreenQuad::ScreenQuad()
{
    vao.Bind();
    vbo.Bind();

    std::vector<ScreenVertex> vertices;
    
    vertices.emplace_back(ScreenVertex{ {-1.f, 1.f,},{0.f,0.f} });
    vertices.emplace_back(ScreenVertex{ {-1.f,-1.f,},{0.f,1.f} });
    vertices.emplace_back(ScreenVertex{ { 1.f,-1.f,},{1.f,1.f} });
    
    vertices.emplace_back(ScreenVertex{ { 1.f,-1.f,},{1.f,1.f} });
    vertices.emplace_back(ScreenVertex{ { 1.f, 1.f,},{1.f,0.f} });
    vertices.emplace_back(ScreenVertex{ {-1.f, 1.f,},{0.f,0.f} });

    vao.LinkVBO(vbo, 0, 2, GL_FLOAT, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, pos));
    vao.LinkVBO(vbo, 1, 2, GL_FLOAT, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, texUV));
    
    vbo.BufferData(vertices);
}

void ScreenQuad::Render()
{
    vao.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
