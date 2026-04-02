#include "GPUAllocator.h"
#include <glad/glad.h>

#include "voxel.h"
#include <iostream>

GPUAllocator::GPUAllocator(size_t byteSizePerType, size_t dataCount)
{
    poolSize = std::max(dataCount,size_t(2));
    dataTypeByteSize = byteSizePerType;

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, dataTypeByteSize * poolSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer);

}

GPUAllocator::~GPUAllocator()
{
    glDeleteBuffers(1, &buffer);
}

GPUAllocation GPUAllocator::Allocate(size_t count,const void* data)
{
    if (cursor + count > poolSize) {
        //https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCopyBufferSubData.xhtml

        size_t newSize = poolSize;

        while (newSize < cursor + count)
            newSize *= 2;


        GLuint newBuffer;
        glGenBuffers(1, &newBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, dataTypeByteSize * newSize, nullptr, GL_DYNAMIC_DRAW);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_COPY_READ_BUFFER, buffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, cursor * dataTypeByteSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDeleteBuffers(1, &buffer);
        
        buffer = newBuffer;
        poolSize = newSize;

        std::cout << "Couldn't allocate GPU Memory Max Size:" 
            << poolSize << "\nTried to allocate\t" << count << ", resized buffer succesfully\n";
    }
    GPUAllocation alloc{ cursor, count };
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, cursor * dataTypeByteSize, count * dataTypeByteSize, data);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    cursor += count;
    return alloc;
}

void GPUAllocator::Bind(uint index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer);
}

void GPUAllocator::Upload(const GPUAllocation& alloc,const void* data)
{
    if (alloc.size > poolSize) {
        cursor = 0;
        Allocate(alloc.size, nullptr);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, alloc.offset * dataTypeByteSize, alloc.size * dataTypeByteSize, data);
}



void GPUAllocator::Free(const GPUAllocation&)
{
    // internal thing make memory available for others
}
