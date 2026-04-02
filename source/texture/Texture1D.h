#pragma once

class Shader;
#include "../engine/Common.h"
class Texture1D
{
public:
    Texture1D(const char* directory, const char* path);
    Texture1D(uint size, void* data);

    GLuint ID;
    int width;
    ~Texture1D() { Delete(); };

    void Bind(int slot);
    void Unbind();
    void Delete();
};

