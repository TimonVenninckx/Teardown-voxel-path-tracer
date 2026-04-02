#pragma once

#include "../engine/Common.h"
class Shader;

class Texture2DArray
{
public:
    GLuint ID;

    int width;
    int height;
    int depth;
    bool mipMapping{ true };

    Texture2DArray(int width, int height,int depth, bool enableMipMaps = true);
    //GLenum type;                 //GL_TEXTURE_2D
    // inputs:         "image",   "type"  "GL_TEXTURE0"  
    void LoadTexture(const char* directory, const char* path, const int depth, bool flipOnLoad = false);

    void TexUnit(Shader& shader, const char* uniform, int slott);

    void GenerateMipMaps();
    //void CopyFrom(Surface* src);
    void Bind(int slot);
    void Unbind();
    void Delete();
};

