#pragma once

#include <vector>
#include "VoxelVolume.h"
#include "Light.h"
#include "Voxel.h"
#include "GPUAllocator.h"
#include "Sphere.h"
#include "Material.h"
#include <memory>
#include <queue>
#include <mutex>

class Shader;
class ScreenQuad;
class Texture1D;
class Texture2D;
struct Scene;

namespace tinybvh {
    class BVH;
}

namespace TIPHYS {
    class PhysicsWorld;
}

class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();

    void LoadScene(const Scene& scene);
    void LoadModel(const char* fileName, glm::vec3 origin = glm::vec3(0.f));

    void RebuildBVH(float delta);

    void Update(float deltaTime);

    struct TraceResult {
        VoxelVolume* volume{};
        glm::uvec3 voxelPos{0};
    };

    void Hit(const glm::vec3& origin, const glm::vec3& dir, float length = 1e30f);
    TraceResult Trace(const glm::vec3& origin, const glm::vec3& dir, float length = 1e30f);

    const std::size_t GetVolumeCount() const{ return voxelVolumes.size(); }
    const std::size_t GetSphereCount() const{ return spheres.size(); }
    const std::size_t GetLightCount()  const{ return lights.size(); }
    const std::size_t GetTotalVoxels() const{ return totalVoxels; }
    const std::size_t GetFilledVoxels()const{ return filledVoxels; }


    std::vector<Sphere> spheres{ 0 };
    std::vector<std::unique_ptr<VoxelVolume>> voxelVolumes{};
    std::vector<Material> materials{ 1 };

    std::vector<Light> lights{
        Light {
            glm::vec4(150.f,120.f,300.f,0.f),
            glm::vec4(-0.62f,-0.43f,-0.84f,0.f),
            glm::vec4(70000,70000.f,0.f,0.f),
            LightType::PointLight,
        },
        Light {
            glm::vec4(-3.f,20.f,55.f,0.f),
            glm::vec4(0.f,-0.8f,0.f,0.f),
            glm::vec4(0.f,250.f,0.f,0.f),
            LightType::SpotLight,
        },
        Light {
            glm::vec4(0.f,20.f,50.f,0.f),
            glm::vec4(0.f,-0.8f,0.f,0.f),
            glm::vec4(250.f,0.f,0.f,0.f),
            LightType::SpotLight,
        },
    };
    bool alwaysRebuildBVH{ false };
    bool physicsUpdate{ true };

    static constexpr float voxelSize = 0.1f;
    TIPHYS::PhysicsWorld* GetPhysicsWorld() {
        return physicsWorld.get();
    }
private:

    void GenerateSpheres();


    std::unique_ptr<tinybvh::BVH> voxelBVH{};
    std::unique_ptr<tinybvh::BVH> sphereBVH{};

    std::size_t totalVoxels{};
    std::size_t filledVoxels{};

    // current brick index into the allocator.
    int currentBrickIndex{};
    std::unique_ptr<GPUAllocator> voxelAllocator{};

    // store indexes into the bricklist
    std::unique_ptr<GPUAllocator> brickIndexAllocator{};
    std::unique_ptr<GPUAllocator> voxelVolumeAllocator{};
    std::unique_ptr<GPUAllocator> materialAllocator{};

    std::unique_ptr<GPUAllocator> voxelBVHAllocator{};
    std::unique_ptr<GPUAllocator> voxelBVHPrimIndexAllocator{};


    std::unique_ptr<GPUAllocator> sphereAllocator{};
    std::unique_ptr<GPUAllocator> sphereBVHAllocator{};
    std::unique_ptr<GPUAllocator> sphereBVHPrimIndexAllocator{};
    GPUAllocation sphereAllocation{};
    GPUAllocation sphereBVHAllocation{};
    GPUAllocation sphereBVHPrimIndexAllocation{};

    GPUAllocation lightAllocation{};
    GPUAllocation voxelBVHAllocation{};
    GPUAllocation voxelBVHPrimIndexAllocation{};

    std::unique_ptr<GPUAllocator> lightAllocator{};
    std::unique_ptr<TIPHYS::PhysicsWorld> physicsWorld{};

    std::vector<GPUAllocation> voxelVolumeAllocations{};

    void TryAddVolumes();
    VoxelVolume* fracturingVolume{};
    std::thread fractureThread;
    std::atomic<bool> fracturingDone{ false };
    std::unique_ptr<std::vector<VoxelVolume::SplitVolume>> volumesToAdd{};
};

