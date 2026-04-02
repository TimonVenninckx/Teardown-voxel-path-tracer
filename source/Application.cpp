#include "Application.h"
#include <iostream>

#include "engine/Engine.h"
#include "engine/Camera.h"
#include "engine/Shader.h"
#include "engine/Timer.h"

#include <ImReflect.hpp>
#include <fstream>

#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include <vector>
#include <random>

#include "ScreenQuad.h"
#include "texture/Texture1D.h"
#include "texture/Texture2D.h"

#include "engine/Common.h"
#include "Voxel.h"
#include "GPUAllocator.h"

#include "VoxelWorld.h"
#include "VoxelVolume.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "GBuffer.h"

#include "SVGF.h"
#include "CatmullRom.h"

#include "Scene.h"
#include "TIPHYS/PhysicsWorld.h"



/// Bunch of settings

enum class BufferDisplayType {
    PathTraced,
    NormalDepth,
    Position,
    Movement,
    Filter,
    Albedo,
}currentlyDisplaying{};

bool displayCatmulRom{ false };

std::vector<CameraPosRot> saveCatmulRom{};

//constexpr uint TEXTURE_WIDTH = 1920*2, TEXTURE_HEIGHT = 1080*2;
constexpr uint TEXTURE_WIDTH = 1920, TEXTURE_HEIGHT = 1080;
//constexpr uint TEXTURE_WIDTH = 1280, TEXTURE_HEIGHT = 720;
//constexpr uint TEXTURE_WIDTH = 400, TEXTURE_HEIGHT = 300;

const glm::ivec3 dispatches{ 128 * 8,1,1 };
const glm::ivec2 screenDispatch{ (TEXTURE_WIDTH + 31) / 32, (TEXTURE_HEIGHT + 31) / 32 };

int maxDiffuseBounce{ 1 };
int maxBounce{ 10 };
float maxDepth{ 300.f };
float entryOffset{ 0.05f };

bool castGodRays{ false };
bool displayShadowMap{ false };
bool traceShadowRays{ true };
bool traceSpheres{ true };

float temporalAlpha{ 0.01f };
bool waveFront{ true };
bool TAA{ false };
bool Denoising{ true };
float catmullRomTimer{};
bool ghosting{ true };
bool useBlueNoise{ true };

float aperture{ 0.00f };
float focusDistance{ 20.f };

RayTracePlane prevCamPlane{};
RayTracePlane camPlane{};
CatmullRom catmullRom{};

Application::Application()
{
    catmullRom.LoadPoints("assets/camera.bin");
    
    // DONT FORGET TO EXTRACT THE ASSETS FOLDER.

    cam = std::make_unique<Camera>(TEXTURE_WIDTH, TEXTURE_HEIGHT, glm::vec3(9.646f, 10.546f, 62.971f), 45.f, 0.1f, 1000.f);
    //cam   = std::make_unique<Camera>(TEXTURE_WIDTH, TEXTURE_HEIGHT, glm::vec3(6.f, 50.f, 110.f), 45.f, 0.1f, 1000.f);
    //cam = std::make_unique<Camera>(TEXTURE_WIDTH, TEXTURE_HEIGHT, glm::vec3(6.f, 12.f, 18.f), 45.f, 0.1f, 1000.f);
    //cam = std::make_unique<Camera>(TEXTURE_WIDTH, TEXTURE_HEIGHT, glm::vec3(-6.f, 12.f, 55.f), 45.f, 0.1f, 1000.f);

    quadShader = std::make_unique<Shader>("shaders/quadshader.vert", "shaders/quadshader.frag");
    
    taaShader = std::make_unique<Shader>();
    taaShader->InitializeCompute("shaders/taacompute.comp");
    
    computeShader = std::make_unique<Shader>();
    computeShader->InitializeCompute("shaders/trace/compute.comp");
    
    waveFrontStartShader = std::make_unique<Shader>();
    waveFrontStartShader->InitializeCompute("shaders/trace/wavefrontstart_compute.comp");
    waveFrontShader = std::make_unique<Shader>();
    waveFrontShader->InitializeCompute("shaders/trace/wavefront_compute.comp");

    waveFrontShadowShader = std::make_unique<Shader>();
    waveFrontShadowShader->InitializeCompute("shaders/trace/wavefrontshadow_compute.comp");

    shadowMapShader= std::make_unique<Shader>();
    shadowMapShader->InitializeCompute("shaders/trace/shadowmap_compute.comp");

    godrayShader = std::make_unique<Shader>();
    godrayShader->InitializeCompute("shaders/godrays.comp");

    svgf = std::make_unique<SVGF>(TEXTURE_WIDTH,TEXTURE_HEIGHT);

    // compute shader stuff.
    compTexture     = std::make_unique<Texture2D>(TEXTURE_WIDTH, TEXTURE_HEIGHT, TextureType::RGBA32F);
    shadowTexture   = std::make_unique<Texture2D>(TEXTURE_WIDTH, TEXTURE_HEIGHT, TextureType::DEPTH);
    taaTexture      = std::make_unique<Texture2D>(TEXTURE_WIDTH, TEXTURE_HEIGHT, TextureType::RGBA32F);

    currentGBuffer  = std::make_unique<GBuffer>(TEXTURE_WIDTH,TEXTURE_HEIGHT);
    previousGBuffer = std::make_unique<GBuffer>(TEXTURE_WIDTH, TEXTURE_HEIGHT);


    // allocating the ray buffers for the whole screen.
    constexpr int rayInfoSize = 16 * 4;
    constexpr int allocCount  = (TEXTURE_WIDTH * TEXTURE_HEIGHT *rayInfoSize);
    constexpr int headerSize  = 4;

    // first index is integer
    rayReadBuffer       = std::make_unique<GPUAllocator>(sizeof(int), headerSize + allocCount);
    rayWriteBuffer      = std::make_unique<GPUAllocator>(sizeof(int), headerSize + allocCount);
    shadowRayAllocator  = std::make_unique<GPUAllocator>(sizeof(int), headerSize + allocCount);
    
    // read and write indexes use by wavefront path-tracing.
    // these are atomic counters, they are read on the GPU.
    int initialValue[2]{ 0 };
    rayReadBuffer->Allocate(headerSize, initialValue);
    rayWriteBuffer->Allocate(headerSize, initialValue);
    shadowRayAllocator->Allocate(headerSize, initialValue);
    
    rayReadBuffer->Allocate(allocCount, nullptr);
    rayWriteBuffer->Allocate(allocCount, nullptr);
    shadowRayAllocator->Allocate(allocCount, nullptr);


    world = std::make_unique<VoxelWorld>();
    
    //skydomeTexture = std::make_unique<Texture2D>("assets/", "HDR_multi_nebulae_1.hdr", "dontcare", false, false, false, true);
    skydomeTexture = std::make_unique<Texture2D>("assets/", "sky_19.hdr", "dontcare", false, true, false, false);
    //skydomeTexture = std::make_unique<Texture2D>("assets/", "white.png", "dontcare", false, false, false, false);
    //skydomeTexture = std::make_unique<Texture2D>("assets/","black.png","dontcare",false,false,false,false);
    //skydomeTexture = std::make_unique<Texture2D>("assets/","test_background.png","dontcare",false,false,false,false);
    for (int i{ 0 }; i < 10; i++)
        blueNoiseTextures[i] = std::make_unique<Texture2D>("assets/noise/", ("stbn_vec2_2Dx1D_128x128x64_" + std::to_string(i) + ".png").c_str(), "dontcare", false, false);

    screenQuad = std::make_unique<ScreenQuad>();
    prevCamPlane = cam->GetRayTracePlane();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

Application::~Application()
{ // unique ptr moment.
}






void Application::Run(float delta)
{
    // scrolling through bluenoise textures.
    blueNoiseIndex++;
    blueNoiseIndex %= blueNoiseTextureCount;

    bool cameraMoved = UpdateMovement(delta);

    if (delta > 0.3f)
        return;

    if (cameraMoved)
        framesSinceMoved = 0;
    if(TAA)
        framesSinceMoved++;

    //std::cout << "Deltatime Passed" << delta << '\n';
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.3f, 0.3f, 1.f);
    
    glm::vec2 resolution{ static_cast<float>(compTexture->width),static_cast<float>(compTexture->height) };
    static int frameNumber{};
    frameNumber++;

    if (displayCatmulRom) {
        catmullRomTimer += delta;
        const auto catmullResult = catmullRom.GetPoint(catmullRomTimer);
        cam->SetPosition(catmullResult.camPos);
        cam->SetOrientation(catmullResult.camFront);
    }

    world->Update(delta);


    prevCamPlane = camPlane;
    camPlane = cam->GetRayTracePlane();

    compTexture->Clear();
    if (waveFront) {
        RenderWaveFront(frameNumber);
    }
    else {
        RenderCompute(frameNumber);
    }
    if (Denoising) {
        Denoise();
    }
    if (TAA && !displayCatmulRom) {
        TemporalAntiAliasing();
    }
    if (castGodRays) {
        RenderGodRays(frameNumber);
    }


    quadShader->Activate();
    if (displayShadowMap)
        shadowTexture->Bind(0);
    else if (!Denoising) {
        compTexture->Bind(0);
    }
    else {
        switch (currentlyDisplaying) {
        case BufferDisplayType::PathTraced:
            compTexture->Bind(0);
            break;
        case BufferDisplayType::NormalDepth:
            currentGBuffer->normalAndDepthBuffer->Bind(0);
            break;
        case BufferDisplayType::Position:
            currentGBuffer->worldPositionBuffer->Bind(0);
            break;
        case BufferDisplayType::Movement:
            svgf->GetMovementTexture().Bind(0);
            break;
        case BufferDisplayType::Filter:
            svgf->GetFilterTexture().Bind(0);
            break;
        case BufferDisplayType::Albedo:
            currentGBuffer->albedoBuffer->Bind(0);
        }
    }
    quadShader->SetInt("image", 0);
    screenQuad->Render();

    Engine::Instance().debug->RenderDebugObjects(cam->GetMatrix());
    Engine::Instance().debug->UpdateLifeTime(delta);

    std::swap(currentGBuffer, previousGBuffer);
    
    Imgui(delta);
}



// used for wavefront path tracing
void Application::SwapWavefrontRayBuffer() {
    // EMTPY OUT THE BUFFER THAT IS NOT BEING USED.
    // So it can be overwritten
    uint readValue  = 0;
    uint writeValue = 0;
    GPUAllocation readAlloc;
    readAlloc.offset = 0;
    readAlloc.size = 1;

    GPUAllocation writeAlloc;
    writeAlloc.offset = 1;
    writeAlloc.size = 1;

    // resetting the read an write values for A and B
    std::swap(rayReadBuffer, rayWriteBuffer);
    rayReadBuffer->Upload(readAlloc, &readValue);
    rayWriteBuffer->Upload(writeAlloc, &writeValue);
    
    rayReadBuffer->Bind(8);
    rayWriteBuffer->Bind(9);

    // shadow buffer stuff.
    shadowRayAllocator->Upload(readAlloc, &readValue);
    shadowRayAllocator->Upload(writeAlloc, &writeValue);
}

void Application::RenderShadowRays()
{
    if (!traceShadowRays)
        return;

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Shadow Ray");

    waveFrontShadowShader->Activate();
    waveFrontShadowShader->SetInt("voxelVolumeCount", static_cast<int>(world->GetVolumeCount()));
    waveFrontShadowShader->SetInt("sphereCount", static_cast<int>(world->GetSphereCount()));
    waveFrontShadowShader->SetInt("lightCount", static_cast<int>(world->GetLightCount()));
    waveFrontShadowShader->SetBool("traceSpheres", traceSpheres);
    waveFrontShadowShader->SetFloat("EPS", entryOffset);
    waveFrontShadowShader->SetFloat("maxDepth", maxDepth);
    waveFrontShadowShader->SetInt("readImageOut", 0);

    compTexture->Bind(0);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute(dispatches.x, dispatches.y, dispatches.z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glPopDebugGroup();
}




void Application::RenderWaveFront(int frameNumber)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "First-hit shader");

    static std::mt19937 rng{};
    
    std::uniform_int_distribution distibution(0,INT_MAX);


    waveFrontStartShader->Activate();
    waveFrontStartShader->SetVec3("camPos", cam->GetPosition());
    waveFrontStartShader->SetVec3("camFront", cam->GetOrientation());
    waveFrontStartShader->SetVec3("cTopLeft", camPlane.topLeft);
    waveFrontStartShader->SetVec3("cTopRight", camPlane.topRight);
    waveFrontStartShader->SetVec3("cBottomLeft", camPlane.bottomLeft);
    waveFrontStartShader->SetInt("voxelVolumeCount", static_cast<int>(world->GetVolumeCount()));
    waveFrontStartShader->SetInt("sphereCount", static_cast<int>(world->GetSphereCount()));
    waveFrontStartShader->SetInt("lightCount", static_cast<int>(world->GetLightCount()));
    waveFrontStartShader->SetInt("frameRandomSeed", distibution(rng));
    waveFrontStartShader->SetInt("frameNumber", frameNumber);
    waveFrontStartShader->SetBool("traceSpheres", traceSpheres);
    waveFrontStartShader->SetFloat("EPS", entryOffset);
    waveFrontStartShader->SetInt("maxBounce", maxBounce);
    waveFrontStartShader->SetFloat("maxDepth", maxDepth);
    waveFrontStartShader->SetFloat("aperture", aperture);
    waveFrontStartShader->SetFloat("focusDistance", focusDistance);
    waveFrontStartShader->SetBool("useBlueNoise", useBlueNoise);
    waveFrontStartShader->SetInt("palette", 4);
    waveFrontStartShader->SetInt("skydome", 5);
    waveFrontStartShader->SetInt("blueNoise", 6);
    waveFrontStartShader->SetInt("shadowMap", 7);
    compTexture->Bind(0);
    currentGBuffer->normalAndDepthBuffer->Bind(1);
    currentGBuffer->worldPositionBuffer->Bind(2);
    currentGBuffer->albedoBuffer->Bind(3);

    skydomeTexture->Bind(5);
    blueNoiseTextures[blueNoiseIndex]->Bind(6);
    shadowTexture->Bind(7);
    rayReadBuffer->Bind(8);
    rayWriteBuffer->Bind(9);
    shadowRayAllocator->Bind(12);



    SwapWavefrontRayBuffer();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glPopDebugGroup();


    waveFrontShader->Activate();
    waveFrontShader->SetVec3("camPos", cam->GetPosition());
    waveFrontShader->SetVec3("camFront", cam->GetOrientation());
    waveFrontShader->SetVec3("cTopLeft", camPlane.topLeft);
    waveFrontShader->SetVec3("cTopRight", camPlane.topRight);
    waveFrontShader->SetVec3("cBottomLeft", camPlane.bottomLeft);
    waveFrontShader->SetInt("voxelVolumeCount", static_cast<int>(world->GetVolumeCount()));
    waveFrontShader->SetInt("sphereCount", static_cast<int>(world->GetSphereCount()));
    waveFrontShader->SetInt("lightCount", static_cast<int>(world->GetLightCount()));
    waveFrontShader->SetInt("frameRandomSeed", rand());
    waveFrontShader->SetInt("frameNumber", frameNumber);
    waveFrontShader->SetFloat("EPS", entryOffset);
    waveFrontShader->SetInt("maxBounce", maxBounce);
    waveFrontShader->SetInt("maxDiffuseBounce", maxDiffuseBounce);
    waveFrontShader->SetBool("traceSpheres", traceSpheres);
    waveFrontShader->SetFloat("maxDepth", maxDepth);
    waveFrontShader->SetInt("skydome", 2);
    waveFrontShader->SetInt("blueNoise", 3);
    waveFrontShader->SetInt("shadowMap", 4);
    waveFrontShader->SetInt("readImageOut", 0);

    compTexture->Bind(0);
    currentGBuffer->albedoBuffer->Bind(3);
    skydomeTexture->Bind(2);
    blueNoiseTextures[blueNoiseIndex]->Bind(3);
    shadowTexture->Bind(4);
    
    rayReadBuffer->Bind(8);
    rayWriteBuffer->Bind(9);
    shadowRayAllocator->Bind(12);
    
    RenderShadowRays();
    for (int i{ 0 }; i < maxBounce; i++) {

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // don't write shadow/bounce rays to buffer on last bounce.
        waveFrontShader->Activate();
        if (i == maxBounce - 1)
            waveFrontShader->SetBool("lastBounce", true);
        else
            waveFrontShader->SetBool("lastBounce", false);

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Bounce-ray shader");
        SwapWavefrontRayBuffer();
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(dispatches.x, dispatches.y, dispatches.z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glPopDebugGroup();


        RenderShadowRays();
    }
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}


void Application::RenderGodRays(int frameNumber)
{
    glm::vec3 sunDirection = glm::normalize(glm::vec3(world->lights[0].direction.x, world->lights[0].direction.y, world->lights[0].direction.z));
    glm::vec3 sunPosition = world->lights[0].position;

    RayTracePlane sunPlane = RayTracePlane::CreateRayTracePlane(sunPosition, sunDirection, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    RayTracePlane p = cam->GetRayTracePlane();

    shadowMapShader->Activate();
    shadowMapShader->SetVec3("camPos", sunPosition);
    shadowMapShader->SetVec3("cTopLeft", sunPlane.topLeft);
    shadowMapShader->SetVec3("cTopRight", sunPlane.topRight);
    shadowMapShader->SetVec3("cBottomLeft", sunPlane.bottomLeft);

    shadowMapShader->SetInt("voxelVolumeCount", static_cast<int>(world->GetVolumeCount()));
    shadowMapShader->SetInt("frameRandomSeed", rand());
    shadowMapShader->SetInt("frameNumber", frameNumber);
    shadowMapShader->SetFloat("EPS", entryOffset);

    shadowTexture->Bind(0);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    godrayShader->Activate();
    godrayShader->SetVec3("sunPosition", sunPosition);
    godrayShader->SetVec3("sunTopLeft", sunPlane.topLeft);
    godrayShader->SetVec3("sunTopRight", sunPlane.topRight);
    godrayShader->SetVec3("sunBottomLeft", sunPlane.bottomLeft);

    godrayShader->SetVec3("camPos", cam->GetPosition());
    godrayShader->SetVec3("camFront", cam->GetOrientation());
    godrayShader->SetInt("frameNumber", frameNumber);
    godrayShader->SetVec3("cTopLeft", p.topLeft);
    godrayShader->SetVec3("cTopRight", p.topRight);
    godrayShader->SetVec3("cBottomLeft", p.bottomLeft);

    godrayShader->SetInt("frameRandomSeed", rand());
    godrayShader->SetInt("shadowMap", 1);
    godrayShader->SetInt("depthBuffer", 2);
    godrayShader->SetFloat("maxDepth", maxDepth);

    shadowTexture->Bind(1);
    currentGBuffer->normalAndDepthBuffer->Bind(2);
    compTexture->Bind(0);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void Application::RenderCompute(int frameNumber)
{
    
    { // raytrace
        Timer pass1;
        RayTracePlane p = cam->GetRayTracePlane();
        computeShader->Activate();
        computeShader->SetVec3("camPos", cam->GetPosition());
        computeShader->SetVec3("camFront", cam->GetOrientation());
        computeShader->SetVec3("cTopLeft", p.topLeft);
        computeShader->SetVec3("cTopRight", p.topRight);
        computeShader->SetVec3("cBottomLeft", p.bottomLeft);
        computeShader->SetBool("castGodRays", castGodRays);
        computeShader->SetInt("voxelVolumeCount", static_cast<int>(world->GetVolumeCount()));
        computeShader->SetInt("lightCount", static_cast<int>(world->GetLightCount()));
        computeShader->SetInt("frameRandomSeed", rand());
        computeShader->SetInt("frameNumber", frameNumber);
        computeShader->SetFloat("EPS", entryOffset);
        computeShader->SetInt("maxBounce", maxBounce);
        computeShader->SetInt("palette", 1);
        computeShader->SetInt("skydome", 2);
        computeShader->SetInt("blueNoise", 3);
        computeShader->SetInt("shadowMap", 4);
        compTexture->Bind(0);
        skydomeTexture->Bind(2);
        blueNoiseTextures[blueNoiseIndex]->Bind(3);
        shadowTexture->Bind(4);

        glDispatchCompute((uint)TEXTURE_WIDTH / 16, (uint)TEXTURE_HEIGHT / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}


void Application::Denoise()
{
    RayTraceFrustum frustum = prevCamPlane.CreateFrustum();
    svgf->Denoise(*currentGBuffer, *previousGBuffer, screenDispatch, *compTexture,frustum);
}

void Application::TemporalAntiAliasing()
{
    // TEMPORAL ANTI ALIASING
    taaShader->Activate();
    taaShader->SetInt("frameSinceMoved", framesSinceMoved);
    compTexture->Bind(0);
    taaTexture->Bind(1);

    glDispatchCompute(screenDispatch.x, screenDispatch.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Application::ReloadShaders()
{
    computeShader->ReloadCompute();
    waveFrontStartShader->ReloadCompute();
    waveFrontShader->ReloadCompute();
    waveFrontShadowShader->ReloadCompute();
    taaShader->ReloadCompute();
    shadowMapShader->ReloadCompute();
    godrayShader->ReloadCompute();

    svgf->ReloadShaders();

    // reset taa sample count.
    framesSinceMoved = 0;
}



void Application::Imgui(float delta)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();
    if (ImGui::Begin("Metrics", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        static float msHistory[120] = {};
        static int msOffset{ 0 };
        float ms = delta * 1000.f;
        msHistory[msOffset++] = ms;
        msOffset %= 120;

        float fps = 1.f / delta;
        ImGui::Text("FPS: %.f", fps);

        static float avrms = 0.f;
        if (msOffset % 30 == 0) {
            avrms = 0.f;
            for (int i{ 0 }; i < 30; i++)
                avrms += msHistory[(msOffset - i + 120) % 120];
            avrms /= 30.f;
        }
        ImGui::Text("FrameTime: %.1fms", avrms);

        ImGui::PlotLines(
            "##ms_graph",
            msHistory,
            120,
            msOffset,
            nullptr,
            0.0f,
            120.f,
            ImVec2(200, 80)
        );
        ImGui::Text("Voxel Volumes: %i", world->GetVolumeCount());
        ImGui::Text("Total  Voxels: %im", world->GetTotalVoxels()  / 1000000);
        ImGui::Text("Filled Voxels: %im", world->GetFilledVoxels() / 1000000);
        ImGui::Text("Total Spheres: %i", world->GetSphereCount());

        ImGui::Text("TAA sample count: %i", framesSinceMoved);

        ImGui::End();
    }


    if (ImGui::Begin("window")) {
        ImGuiTabBarFlags flags{};
        flags |= ImGuiTabBarFlags_Reorderable;
        flags |= ImGuiTabBarFlags_DrawSelectedOverline;
        flags |= ImGuiTabBarFlags_FittingPolicyShrink;
        
        ImGui::BeginTabBar("Tabs",flags);
        if (ImGui::BeginTabItem("General")){

#ifdef _DEBUG
            ImGui::Text("RUN IT IN RELEASE!!!");
            ImGui::Text("BVH Building and Physics are very slow otherwise!!!");
#endif

            ImGui::Checkbox("LockMouse[F]", &lockMouse);
            ImGui::Checkbox("physics", &world->physicsUpdate);
            ImGui::Checkbox("camera-ghosting", &ghosting);
            

            // we build the scene names once at startup.
            static std::vector<const char*> sceneNames;
            if (sceneNames.empty()) {
                for (const auto& scene : ALLSCENES)
                    sceneNames.push_back(scene.name.c_str());
            }

            static int selectedScene = -1;
            if (ImGui::Combo("Scene", &selectedScene, sceneNames.data(), static_cast<int>(sceneNames.size()))) {
                world->LoadScene(ALLSCENES[selectedScene]);
                TIPHYS::Box box;
                box.halfExtents = glm::vec3(0.3f,1.f,0.3f);
                box.position = glm::vec3(0.f, 100.f, 0.f);
                box.mass = 100.f;
                playerHandle = world->GetPhysicsWorld()->AddBox(box);
            }
            ImGui::Text("Load scene here^^^^^^\n\n");


            ImGui::Text("Press G to break stuff :P");
            ImGui::Text("Hold E to pick up objects");
            ImGui::Text("Hold RightMouseButton to Speed up");


            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Render")) {

            if (ImGui::Checkbox("ShadowRay", &traceShadowRays)) {
                framesSinceMoved = 0;
            }

            ImGui::Checkbox("TraceSphers", &traceSpheres);

            if (ImGui::Checkbox("TAA", &TAA)) {
                framesSinceMoved = 0;
            }
            ImGui::Checkbox("SVGF-Denoising", &Denoising);
            if (ImGui::CollapsingHeader("SVGF-Settings")) {
                ImGui::DragFloat("temporalAlpha", &svgf->temporalAlpha, 0.001f, 0.001f, 1.f);
                ImGui::DragFloat("sigmaz", &svgf->sigmaz, 0.001f, 0.001f, 1.f);
                ImGui::DragFloat("sigman", &svgf->sigman, 1.f, 0.001f, 400.f);
                ImGui::DragFloat("sigmal", &svgf->sigmal, 0.1f, 0.001f, 20.f);
            }

            ImReflect::Input("CurrentlyDisplaying:", currentlyDisplaying);

            if (ImGui::InputInt("maxRayBounce", &maxBounce, 1, 100)) {
                framesSinceMoved = 0;
            }
            if (ImGui::InputInt("maxDiffuseRayBounce", &maxDiffuseBounce, 1, 15)) {
                framesSinceMoved = 0;
            }
            ImGui::Checkbox("castGodRays", &castGodRays);
            ImGui::Checkbox("SunView", &displayShadowMap);

            if (ImGui::Checkbox("WaveFront", &waveFront)) {
                framesSinceMoved = 0;
            }
            ImGui::Checkbox("useBlueNoise", &useBlueNoise);
            if (ImGui::DragInt("blueNoiseTextureCount", &blueNoiseTextureCount, 1, 1, 10)) {
                blueNoiseTextureCount = std::clamp(blueNoiseTextureCount, 1, 10);
            }
            ImGui::DragFloat("EPS", &entryOffset, 0.0001f, 0.000000001f, 0.1f, "%.10f");
            if (ImGui::Button("Reload Shader")) {
                ReloadShaders();
            }
            if (ImGui::Button("GetTotalTextureColorValue")) {
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                compTexture->GetTotalColorValue();
            }

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("Lights")) {
            for (int i{ 0 }; i < world->lights.size(); i++) {

                ImGui::PushID(i);
                if (ImGui::CollapsingHeader((std::string("light") + std::to_string(i)).c_str())) {
                    Light& light = world->lights[i];
                    ImReflect::Input("Type", light.type);
                    if(ImGui::DragFloat3("Direction", &light.direction[0], 0.01f, -1.f, 1.f)
                        || ImGui::DragFloat3("Position", &light.position[0])
                        || ImGui::ColorEdit3("Color", &light.color[0])
                        || ImGui::DragFloat("InnerCutOff", &light.innercutOff, 0.01f, 0.f, 1.f)
                        || ImGui::DragFloat("OuterCutOff", &light.outerCutOff, 0.01f, 0.f, 1.f)
                        )
                        framesSinceMoved = 0;
                }
                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Materials")) {
            for (int i{ 1 }; i < world->materials.size(); i++) {
                
                ImGui::PushID(i);
                if (ImGui::CollapsingHeader((std::string("material") + std::to_string(i)).c_str())) {
                    Material& mat = world->materials[i];
                    ImReflect::Input("Type", mat.type);
                    if (ImGui::ColorEdit3("Color", &mat.color[0])
                        || ImGui::DragFloat("Emittance", &mat.emit, 0.01f, 0.f, 1.f)
                        || ImGui::DragFloat("Flux", &mat.flux, 0.01f, 0.f, 1.f)
                        || ImGui::DragFloat("Ior", &mat.ior, 0.01f, 0.f, 1.f)
                        )
                        framesSinceMoved = 0;
                }
                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Camera")) {

            glm::vec3 pos = cam->GetPosition();
            if (ImGui::DragFloat3("position", &pos[0]))
                cam->SetPosition(pos);
            ImGui::DragFloat("moveSpeed", &moveSpeed);

            if (ImGui::DragFloat("aperture", &aperture, 0.0001f, 0.f, 10.f)) {
                framesSinceMoved = 0;
            }
            if (ImGui::DragFloat("focusDistance", &focusDistance, 0.1f, 0.f, 10000.f)) {
                framesSinceMoved = 0;
            }

            ImGui::Checkbox("Catmullrom", &displayCatmulRom);

            ImGui::DragFloat("Catmull-Tension", &catmullRom.tension, 0.01f, 0.f, 1.f);
            ImGui::DragFloat("Catmull-Alpha", &catmullRom.alpha, 0.01f, 0.f, 1.f);

            if (ImGui::Button("ResetCatmullRom")) {
                catmullRomTimer = 0.f;
            }

            if (ImGui::Button("SaveCam")) {
                saveCatmulRom.emplace_back(CameraPosRot{ cam->GetPosition(), cam->GetOrientation() });
            }

            if (ImGui::Button("UpdateCatMullRom")) {
                if (saveCatmulRom.size() > 3)
                    catmullRom.LoadPoints(saveCatmulRom);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Application::UpdateMovement(float delta)
{
    bool moved = false;

    Input& i = *Engine::Instance().input;

    constexpr glm::vec3 up{ 0.f,1.f,0.f };
    glm::vec3 front = cam->GetOrientation();
    glm::vec3 side = glm::cross(front, up);

    // player is ghosting
    if (ghosting) {
        glm::vec3 pos = cam->GetPosition();

        const float speed = delta * moveSpeed * (i.IsMouseDown(MouseButton::RIGHT) ? 10.f : 1.f);

        if (i.IsKeyDown(Key::W)) pos += front * speed, moved = true;
        if (i.IsKeyDown(Key::S)) pos -= front * speed, moved = true;
        if (i.IsKeyDown(Key::D)) pos += side * speed, moved = true;
        if (i.IsKeyDown(Key::A)) pos -= side * speed, moved = true;

        if (i.IsKeyDown(Key::SPACE))      pos.y += speed, moved = true;
        if (i.IsKeyDown(Key::SHIFT_LEFT)) pos.y -= speed, moved = true;

        cam->SetPosition(pos);
    } 
    // using an AABB as player collision, a bit scuffed tbh.
    // since there is no perfect voxel-collision
    else if(TIPHYS::Box* player = playerHandle.Get()) {

        glm::vec3 pos = player->position;

        glm::vec3 localfront = glm::normalize(glm::vec3(front.x, 0.f, front.z));
        glm::vec3 localside  = glm::normalize(glm::vec3(side.x, 0.f, side.z));

        glm::vec3 velocity{};
        const float speed = moveSpeed * (i.IsMouseDown(MouseButton::RIGHT) ? 10.f : 1.f);


        if (i.IsKeyDown(Key::W)) velocity += localfront * speed, moved = true;
        if (i.IsKeyDown(Key::S)) velocity -= localfront * speed, moved = true;
        if (i.IsKeyDown(Key::D)) velocity += localside * speed, moved = true;
        if (i.IsKeyDown(Key::A)) velocity -= localside * speed, moved = true;

        if (i.IsKeyDown(Key::SPACE)) velocity.y += delta * speed * 3, moved = true;

        player->velocity = glm::vec3(velocity.x, player->velocity.y + velocity.y, velocity.z);
        cam->SetPosition(pos + glm::vec3(0.f,1.f,0.f));
    }

    if (i.IsKeyPressed(Key::F)) lockMouse = !lockMouse;

    if (i.IsKeyPressed(Key::G)) {
        world->Hit(cam->GetPosition(), cam->GetOrientation());
    }

    if (i.IsKeyPressed(Key::E)) {
        playerHeldVolume = world->Trace(cam->GetPosition(), cam->GetOrientation()).volume;
        if (playerHeldVolume) {
            playerHeldVolume->SetPhysicsStatic(false);
            playerHeldVolumeArmLength = glm::length(cam->GetPosition() - playerHeldVolume->GetPosition());
        }
    }
    if (i.IsKeyRelease(Key::E)) playerHeldVolume = nullptr;

    if (playerHeldVolume) {
        // -1 | 0 | 1 are possible values
        float scrollDelta = static_cast<float>(i.GetScrollWheelDelta()) * 0.1f;
        playerHeldVolumeArmLength = std::clamp(playerHeldVolumeArmLength * (1.f + scrollDelta),1.f,1000.f);

        playerHeldVolume->SetPosition(cam->GetPosition() + cam->GetOrientation() * playerHeldVolumeArmLength);
        world->RebuildBVH(0.f);
    }


    // first time mouse can move intensly.
    static bool firstTime{ true };
    glm::vec2 mouseDelta = i.GetMouseDelta();
    mouseDelta *= 0.01f;
    if (!lockMouse) {
        if (abs(mouseDelta.x) > 0.000001f || abs(mouseDelta.y) > 0.000001f) {
            moved = true;
            if (firstTime) {
                firstTime = false;
            }
            else {
                // ye camera is a bit off a mess
                front = glm::rotate(front, mouseDelta.y, -side);
                front = glm::rotate(front, mouseDelta.x, -up);

                cam->SetOrientation(front);
            }
        }
    }

    return moved;
}