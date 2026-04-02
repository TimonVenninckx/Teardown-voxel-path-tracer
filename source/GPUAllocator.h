#pragma once

#include "engine/common.h"

struct GPUAllocation {
    size_t offset{0};
    size_t size{0};
};

class GPUAllocator
{
public:
    // sizeof(DataType) and How many of the data Type.
    GPUAllocator(size_t byteSizePerType, size_t dataCount = 2);
    ~GPUAllocator();

    // allocate count * dataTypeByteSize bytes
    GPUAllocation Allocate(size_t count,const void* data);
    // bind GL_SHADER_STORAGE_BUFFER
    void Bind(uint index);
    // upload data to gpu.
    void Upload(const GPUAllocation& alloc,const void* data);

    // doesn't do stuff yet, could add a free-list.
    // something like a vector of open slots,
    // or something like from slot-start ... until length ... 
    // there is empty space in the buffer
    void Free(const GPUAllocation& alloc);

    // reset the buffer, doesn't deallocate
    // since we probably need to same amount of data
    // for the next time.
    void Reset() { cursor = 0; }

private:
    GLuint buffer{};
    size_t dataTypeByteSize;

    // poolSize is data Count
    // actual size = PoolSize * dataTypeByteSize
    size_t poolSize{2};
    size_t cursor{};
};

