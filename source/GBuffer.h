#pragma once
#include "engine/Common.h"

#include <memory>

class Texture2D;
class GPUAllocator;
class GBuffer {
public:

    GBuffer(uint width, uint height);

    // stores normal.xyz and depth.w
    std::unique_ptr<Texture2D> normalAndDepthBuffer{};
    // world position.xyz
    std::unique_ptr<Texture2D> worldPositionBuffer{};
    // moments
    std::unique_ptr<Texture2D> momentsBuffer{};

    std::unique_ptr<Texture2D> albedoBuffer{};

    //std::unique_ptr<GPUAllocator> voxelIDBuffer;
};
