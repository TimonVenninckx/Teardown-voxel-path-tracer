#pragma once

#include "buffers/VAO.h"
#include "buffers/VBO.h"


class ScreenQuad
{
public:
    ScreenQuad();
    void Render();

private:
    VAO vao;
    VBO vbo;
};

