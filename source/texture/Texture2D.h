#pragma once

#include <string>
#include "../engine/Common.h"
class Shader;

enum class TextureType {
    DEFAULT,
    RGBA32F,
    RGBA16F,
    RGBA16FX3,

    DEPTH,
};

class Texture2D
{
public:
    GLuint ID;
    //const char* type;
    //const char* path;
    std::string type;
    std::string path;
    TextureType textureType{ TextureType::DEFAULT };
    int width;
    int height;
    
    //GLenum type;                 //GL_TEXTURE_2D
    // inputs:         "image",   "type"  "GL_TEXTURE0"  
    Texture2D(const char* directory, const char* path, const char* type = "diffuse", bool useMipMapping = true, bool linearSampling = true, bool flipTexture = false, bool hdr = false);

    // create depth component texture with width and height;
    Texture2D(unsigned int width, unsigned int height, TextureType type = TextureType::RGBA32F);
    ~Texture2D() { Delete();};

    void TexUnit(Shader& shader, const char* uniform,int slott);

    float GetTotalColorValue();
    
    void Clear();

    void Bind(int slot)const;
    void Unbind();
    void Delete();
};

