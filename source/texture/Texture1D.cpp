#include "Texture1D.h"
#include <stdexcept>
#include "../engine/Shader.h"
#include <glad/glad.h>
#include <stb_image.h>



Texture1D::Texture1D(const char* directory, const char* path)
{
    //this->path = path;
    //this->type = texType;

    //printf("Creating texture:)\n");
    std::string p = directory;
    if (p.length() > 0)
        p.append("/");
    p.append(path);

    int height;
    int numColCh;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load(p.c_str(), &width, &height, &numColCh, 0);

    // we just make it 1d if there are multiple dimensions;
    width *= height;

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_1D, ID);

    //if (useMipMapping)
    //    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //else if (linearSampling)
    //    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //else
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (numColCh == 4)
        glTexImage1D
        (
            GL_TEXTURE_1D,
            0,
            GL_RGBA,
            width,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else if (numColCh == 3)
        glTexImage1D
        (
            GL_TEXTURE_1D,
            0,
            GL_RGB,
            width,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else if (numColCh == 1)
        glTexImage1D
        (
            GL_TEXTURE_1D,
            0,
            GL_RED,
            width,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else
        throw std::invalid_argument("Automatic texture type recognition failed");


    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_1D, 0);
}

Texture1D::Texture1D(uint size, void* data)
{
    width = size;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_1D, ID);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage1D
    (
        GL_TEXTURE_1D,
        0,
        GL_RGBA,
        width,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glBindTexture(GL_TEXTURE_1D, 0);
}


void Texture1D::Bind(int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_1D, ID);
}

void Texture1D::Unbind()
{
    glBindTexture(GL_TEXTURE_1D, 0);
}
void Texture1D::Delete()
{
    glDeleteTextures(1, &ID);
}
