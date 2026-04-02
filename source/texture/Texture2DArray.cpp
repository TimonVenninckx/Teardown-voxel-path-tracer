#include "Texture2DArray.h"
#include <iostream>

#include <glad/glad.h>
#include <stb_image.h>
#include "../engine/Shader.h"



Texture2DArray::Texture2DArray(int width, int height, int depth, bool enableMipMaps)
    : width(width)
    , height(height)
    , depth(depth)
    , mipMapping(enableMipMaps)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


    if (enableMipMaps) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, 2);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_LOD, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, 0);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

}

//  GLenum format, GLenum pixelType
void Texture2DArray::LoadTexture(const char* directory, const char* path, const int index, bool flipOnLoad)
{
    std::string p = directory;
    if (p.length() > 0)
        p.append("/");
    p.append(path);
    
    stbi_set_flip_vertically_on_load(flipOnLoad);

    int tWidth;
    int tHeight;
    int numColCh;
    unsigned char* bytes = stbi_load(p.c_str(), &tWidth, &tHeight, &numColCh, STBI_rgb_alpha);

    //assert(tWidth == width);
    //assert(tHeight == height);

    if (!bytes) {
        std::cout << "Texture2DArray::Couldn't load file:" << p << std::endl;
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    //                                      texture.second                
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::TexUnit(Shader& shader, const char* uniform, int slot)
{
    shader.Activate();
    const GLint texUni = glGetUniformLocation(shader.ID, uniform);
    glUniform1i(texUni, slot);

}

void Texture2DArray::GenerateMipMaps()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}



void Texture2DArray::Bind(int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}

void Texture2DArray::Unbind()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
void Texture2DArray::Delete()
{
    glDeleteTextures(1, &ID);
}
