#include "SVGF.h"
#include "engine/Shader.h"
#include "GBuffer.h"
#include "texture/Texture2D.h"
#include "glad/glad.h"
#include "engine/Camera.h"

SVGF::SVGF(uint textureWidth, uint textureHeight)
{
    this->textureWidth = textureWidth;
    this->textureHeight = textureHeight;

    movementVectorShader = std::make_unique<Shader>();
    atrousShader = std::make_unique<Shader>();
    remodulateShader = std::make_unique<Shader>();
    estimateVarianceShader = std::make_unique<Shader>();

    movementVectorShader->InitializeCompute("shaders/svgf/movementvector_compute.comp");
    atrousShader->InitializeCompute("shaders/svgf/atrous_compute.comp");
    remodulateShader->InitializeCompute("shaders/svgf/remodulate_compute.comp");
    estimateVarianceShader->InitializeCompute("shaders/svgf/estimatevariance_compute.comp");

    movementVectorTexture  = std::make_unique<Texture2D>(textureWidth, textureHeight, TextureType::RGBA16F);
    filterTextureA = std::make_unique<Texture2D>(textureWidth, textureHeight, TextureType::RGBA16F);
    filterTextureB = std::make_unique<Texture2D>(textureWidth, textureHeight, TextureType::RGBA16F);
    taaTextureA = std::make_unique<Texture2D>(textureWidth, textureHeight, TextureType::RGBA16F);
    taaTextureB = std::make_unique<Texture2D>(textureWidth, textureHeight, TextureType::RGBA16F);
}


void SVGF::ReloadShaders()
{
    movementVectorShader->ReloadCompute();
    atrousShader->ReloadCompute();
    estimateVarianceShader->ReloadCompute();
    remodulateShader->ReloadCompute();
}

void SVGF::CreateMotionVectors(const GBuffer& curGBuffer, const GBuffer& prevGBuffer, const glm::ivec2& screenDispatch, const Texture2D& outputTexture, const RayTraceFrustum& frustum)
{
    

    // movement vector
    movementVectorShader->Activate();
    movementVectorShader->SetVec4("left", frustum.left);
    movementVectorShader->SetVec4("right", frustum.right);
    movementVectorShader->SetVec4("top", frustum.top);
    movementVectorShader->SetVec4("bottom", frustum.bottom);
    movementVectorShader->SetFloat("temporalAlpha", temporalAlpha);
    movementVectorShader->SetInt("displayTexture", 10);
    movementVectorShader->SetInt("worldPositionTexture", 3);
    movementVectorShader->SetInt("previousMomentsTexture", 4);
    movementVectorShader->SetInt("previousNormalTexture", 6);
    movementVectorShader->SetInt("normalTexture", 7);
    movementVectorShader->SetInt("albedoTexture", 8);
    movementVectorShader->SetInt("taaTexture", 9);


    movementVectorTexture->Bind(0);
    if (taaReadFromA) {
        taaTextureA->Bind(9);
        taaTextureB->Bind(2);
    }
    else {
        taaTextureA->Bind(2);
        taaTextureB->Bind(9);
    }
    taaReadFromA = !taaReadFromA;
    curGBuffer.worldPositionBuffer->Bind(3);
    prevGBuffer.momentsBuffer->Bind(4);
    curGBuffer.momentsBuffer->Bind(5);
    prevGBuffer.normalAndDepthBuffer->Bind(6);
    curGBuffer.normalAndDepthBuffer->Bind(7);
    curGBuffer.albedoBuffer->Bind(8);
    outputTexture.Bind(10);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    estimateVarianceShader->Activate();
    estimateVarianceShader->SetInt("momentsHistTexture", 1);
    estimateVarianceShader->SetInt("colorTexture", 2);
    estimateVarianceShader->SetInt("normalTexture", 3);

    filterTextureA->Bind(0);
    curGBuffer.momentsBuffer->Bind(1);
    (taaReadFromA ? taaTextureA : taaTextureB)->Bind(2);
    curGBuffer.normalAndDepthBuffer->Bind(3);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void SVGF::Denoise(const GBuffer& curGBuffer,const GBuffer& prevGBuffer,const glm::ivec2& screenDispatch, const Texture2D& outputTexture, const RayTraceFrustum& frustum)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SVGF-Denoising");

    CreateMotionVectors(curGBuffer, prevGBuffer, screenDispatch, outputTexture, frustum);


    atrousShader->Activate();
    atrousShader->SetInt("readTexture", 1);

    atrousShader->SetInt("normalAndDepthTexture", 3);
    atrousShader->SetInt("momentsTexture", 4);

    atrousShader->SetFloat("sigmaz", sigmaz);
    atrousShader->SetFloat("sigman", sigman);
    atrousShader->SetFloat("sigmal", sigmal);
    curGBuffer.normalAndDepthBuffer->Bind(3);
    curGBuffer.momentsBuffer->Bind(4);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);


    // A has the filterData from
    // estimatevariance shader
    // always read from filterTexture A the first time

    bool writeA{ false};
    for (int i{ 0 }; i < 5; i++) {
        writeA = (i % 2);
        if (writeA) {
            filterTextureA->Bind(0);
            filterTextureB->Bind(1);
        }
        else {
            filterTextureA->Bind(1);
            filterTextureB->Bind(0);
        }

        atrousShader->SetInt("iteration", i);

        glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        if (i == 0) {
            glCopyImageSubData(
                filterTextureB->ID, GL_TEXTURE_2D, 0, 0, 0, 0,
                (taaReadFromA ? taaTextureA : taaTextureB)->ID, GL_TEXTURE_2D, 0, 0, 0, 0,
                textureWidth, textureHeight,1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    remodulateShader->Activate();
    remodulateShader->SetInt("albedoTexture", 1);
    remodulateShader->SetInt("filterTexture", 2);


    outputTexture.Bind(0);
    curGBuffer.albedoBuffer->Bind(1);
    (writeA ? filterTextureA : filterTextureB)->Bind(2);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glPopDebugGroup();
}

Texture2D& SVGF::GetMovementTexture() {
    return *movementVectorTexture;
}


Texture2D& SVGF::GetFilterTexture()
{
    return taaReadFromA ? *taaTextureA  : *taaTextureB;
}

