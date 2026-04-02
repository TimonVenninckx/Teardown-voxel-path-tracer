#include "GBuffer.h"
#include "texture/Texture2D.h"
#include "GPUAllocator.h"


GBuffer::GBuffer(uint width, uint height)
{
    normalAndDepthBuffer= std::make_unique<Texture2D>(width, height, TextureType::RGBA32F);
    worldPositionBuffer = std::make_unique<Texture2D>(width,height, TextureType::RGBA32F);
    momentsBuffer       = std::make_unique<Texture2D>(width, height, TextureType::RGBA16F);
    albedoBuffer        = std::make_unique<Texture2D>(width, height, TextureType::RGBA16F);
    //voxelIDBuffer       = std::make_unique<GPUAllocator>(sizeof(uint), width * height);
}
