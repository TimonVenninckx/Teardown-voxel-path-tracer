#include "Texture2D.h"

#include <stdexcept>
#include "../engine/Shader.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <vector>

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            //case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            //case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

//  GLenum format, GLenum pixelType
Texture2D::Texture2D(const char* directory, const char* path, const char* texType, bool useMipMapping, bool linearSampling, bool flipTexture,bool hdr)
{
    this->path = path;
    this->type = texType;

    //printf("Creating texture:)\n");
    std::string p = directory;
    if(p.length() > 0)
        p.append("/");
    p.append(path);





    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    if(useMipMapping)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else if(linearSampling)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    

    int numColCh;
    stbi_set_flip_vertically_on_load(flipTexture);
    if (hdr) {
        float* data = stbi_loadf(p.c_str(), &width, &height, &numColCh, 0);


        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB16F,
            width, height,
            0,
            GL_RGB,
            GL_FLOAT,
            data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return;

    }
    unsigned char* bytes = stbi_load(p.c_str(), &width, &height, &numColCh, 0);


    if (numColCh == 4)
        glTexImage2D
        (
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else if (numColCh == 3)
        glTexImage2D
        (
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else if (numColCh == 1)
        glTexImage2D
        (
            GL_TEXTURE_2D,
            0,
            GL_RED,
            width,
            height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            bytes
        );
    else
        throw std::invalid_argument("Automatic texture type recognition failed");

    if(useMipMapping)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);
}



Texture2D::Texture2D(unsigned int width, unsigned int height,TextureType type)
{
    this->width = width;
    this->height = height;
    this->textureType = type;
    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    switch (textureType) {
    case TextureType::RGBA32F:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        break;
    case TextureType::RGBA16F:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        break;
    case TextureType::DEPTH:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
        glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        break;
    default:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        float borderColor[]{ 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }



    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::TexUnit(Shader& shader, const char* uniform, int slot)
{
    shader.Activate();
    const GLint texUni = glGetUniformLocation(shader.ID, uniform);
    glUniform1i(texUni, slot);
}

float Texture2D::GetTotalColorValue()
{
    struct vec4 { float x{}, y{}, z{}, w{}; };
    vec4 total{};
    std::vector<vec4> pixels(width * height);
    switch (textureType) {
    case TextureType::RGBA32F:
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        //glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D,ID);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        break;
    }
    for (vec4& p : pixels) {
        total.x = total.x + p.x;
        total.y = total.y + p.y;
        total.z = total.z + p.z;
        total.w = total.w + p.w;
    }
    
    std::cout << "Red  : " << total.x << '\t';
    std::cout << "Green: " << total.y << '\t';
    std::cout << "Blue : " << total.z << '\t';
    std::cout << "Alpha: " << total.w << '\n';

    return total.x + total.y + total.z + total.w;
}


void Texture2D::Clear() {
    GLenum format = GL_RGBA;
    GLenum formatType = GL_FLOAT;

    switch (textureType) {
    case TextureType::RGBA32F:
    case TextureType::RGBA16F:
        format = GL_RGBA;
        break;
    case TextureType::DEPTH:
        format = GL_RED;
        break;
    default:
        format = GL_DEPTH_COMPONENT;
        break;
    }

    glClearTexImage(ID, 0, format, formatType, NULL);
}

void Texture2D::Bind(int slot)const
{
    glActiveTexture(GL_TEXTURE0 + slot);

    switch (this->textureType) {
    case TextureType::RGBA32F:
        glBindImageTexture(slot, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        break;
    case TextureType::RGBA16F:
        glBindImageTexture(slot, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        break;
    case TextureType::DEPTH:
        glBindImageTexture(slot, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        break;
    }
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture2D::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture2D::Delete()
{
    glDeleteTextures(1, &ID);
}
