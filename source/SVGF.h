#pragma once

#include "engine/Common.h"
#include <glm/vec2.hpp>
#include <memory>


class Shader;
class Texture2D;
class GBuffer;
struct RayTraceFrustum;


class SVGF
{
public:

    SVGF(uint textureWidth, uint textureHeight);

    void ReloadShaders();

    void Denoise(const GBuffer& curGBuffer, const GBuffer& prevGBuffer,const glm::ivec2& screenDispatch, const Texture2D& outputTexture, const RayTraceFrustum& frustum);

    Texture2D& GetMovementTexture();
    Texture2D& GetFilterTexture();

    float temporalAlpha{ 0.15f };

    float sigmaz{1.f};
    float sigman{128.f};
    float sigmal{4.f};


private:
    void CreateMotionVectors(const GBuffer& curGBuffer, const GBuffer& prevGBuffer, const glm::ivec2& screenDispatch,const Texture2D& outputTexture, const RayTraceFrustum& frustum);

    bool taaReadFromA{ true };

    uint textureWidth{};
    uint textureHeight{};

    std::unique_ptr<Shader> movementVectorShader{};
    std::unique_ptr<Shader> atrousShader{};
    std::unique_ptr<Shader> estimateVarianceShader{};
    std::unique_ptr<Shader> remodulateShader{};



    std::unique_ptr<Texture2D> taaTextureA{};
    std::unique_ptr<Texture2D> taaTextureB{};
    std::unique_ptr<Texture2D> movementVectorTexture{};
    std::unique_ptr<Texture2D> filterTextureA{};
    std::unique_ptr<Texture2D> filterTextureB{};
};

