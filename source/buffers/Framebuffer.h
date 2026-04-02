#pragma once

class CubeMap;

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    void Bind();
    void Unbind();

    unsigned int ID;
};

