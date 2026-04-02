#pragma once

#include <vector>
#include <memory>

class Camera;
class Shader;
class ScreenQuad;
class Texture1D;
class Texture2D;
class VoxelWorld;
class GPUAllocator;
class GBuffer;
class SVGF;
class VoxelVolume;

// ugly include
#include "TIPHYS/Body.h"

class Application
{
public:

    Application();
    ~Application();

    // press F to lock/unlock
    bool lockMouse{ true };
    float moveSpeed{ 5.f };
    int framesSinceMoved{ 0 };


    void Run(float deltaime);

private:
    // swap the ray read and write buffers
    void SwapWavefrontRayBuffer();
    
    // render shadow-rays when doing wavefront-path tracing
    void RenderShadowRays();
    
    // wavefront style path-tracing 
    // https://jacco.ompf2.com/2019/07/18/wavefront-path-tracing/
    void RenderWaveFront(int frameNumber);
    
    // rendering to shadowmap from light[0]
    // then using raymarching from camera to depthbuffer
    // and plotting points onto the shadomap
    void RenderGodRays(int framenumber);

    // classic single shader path tracer
    // doesn't have all the features.
    void RenderCompute(int frameNumber);

    // call the SVGF denoiser
    void Denoise();

    // just an accumulator, TAA only works with the standard path tracer.
    // as that one jitters the ray a tiny bit in each pixel.
    // simulating high SPP over time. This doesn't work with denoising.
    void TemporalAntiAliasing();
    
    
    void ReloadShaders();

    void Imgui(float delta);

    std::unique_ptr<Shader> computeShader{};
    std::unique_ptr<Shader> waveFrontStartShader{};
    std::unique_ptr<Shader> waveFrontShader{};
    std::unique_ptr<Shader> waveFrontShadowShader{};
    std::unique_ptr<Shader> godrayShader{};


    std::unique_ptr<Shader> taaShader{};
    std::unique_ptr<Shader> shadowMapShader{};
    std::unique_ptr<Shader> quadShader{};
    
    std::unique_ptr<SVGF> svgf;

    std::unique_ptr<Camera> cam{};
    std::unique_ptr<ScreenQuad> screenQuad{};
 

    std::unique_ptr<Texture2D> skydomeTexture{};
    uint blueNoiseIndex{};
    int blueNoiseTextureCount{ 4 };
    std::unique_ptr<Texture2D> blueNoiseTextures[10]{};

    std::unique_ptr<Texture2D> taaTexture{};
    std::unique_ptr<Texture2D> compTexture{};
    std::unique_ptr<Texture2D> shadowTexture{};
    
    // G-Buffer
    std::unique_ptr<GBuffer> currentGBuffer{};
    std::unique_ptr<GBuffer> previousGBuffer{};

    std::unique_ptr<GPUAllocator> rayReadBuffer{};
    std::unique_ptr<GPUAllocator> rayWriteBuffer{};
    std::unique_ptr<GPUAllocator> shadowRayAllocator{};

    VoxelVolume* playerHeldVolume{};
    float playerHeldVolumeArmLength{};

    std::unique_ptr<VoxelWorld> world{};

    TIPHYS::PhysicsBodyHandle playerHandle{};

    // return if player moved, used for accumulator
    bool UpdateMovement(float delta);
};

