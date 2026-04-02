#include "VBO.h"
#include "glad/glad.h"


VBO::VBO()
{
}

VBO::~VBO()
{
    Delete();
}

inline void VBO::Generate()
{
    // not initalised
    if (ID == 0) {
        glGenBuffers(1, &ID);
    }
}

void VBO::ActuallyBufferData(void* data, std::size_t dataSizeInBytes)
{
    Generate();
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, data, GL_STATIC_DRAW);
}

void VBO::Bind()
{
    Generate();
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        Unbind();
    }
}


