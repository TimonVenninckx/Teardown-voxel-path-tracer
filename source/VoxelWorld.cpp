#include "VoxelWorld.h"

#include "engine/Engine.h"
#include "engine/Camera.h"
#include "engine/Shader.h"
#include "engine/Timer.h"

#include <ImReflect.hpp>


#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include <vector>
#include <random>
#include "texture/Texture1D.h"
#include "texture/Texture2D.h"

#include "engine/Common.h"
#include "GPUAllocator.h"
#include "MagicaParser.h"
#include <random>

#define ENABLE_CUSTOM_GEOMETRY
#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"


#include "Scene.h"
#include "TIPHYS/PhysicsWorld.h"
#include <thread>


std::vector<std::unique_ptr<VoxelVolume>>* g_volumes;

glm::vec3 ToVec3(const tinybvh::bvhvec3& v) {
    return {v.x, v.y, v.z};
}

bool CustomIntersect(tinybvh::Ray& ray, const unsigned primID) {

    if (!g_volumes) {
        std::cout << "No voxel volumes in custom intersect\n";
        return false;
    }
    if (primID >= g_volumes->size()) {
        std::cout << "PrimID" << primID << " too high.\n";
        return false;
    }

    VoxelVolume::IntersectResult result = (*g_volumes)[primID]->Intersect(ToVec3(ray.O), ToVec3(ray.D));

    std::cout << "Intersecting Volume: ID:\n";

    if (result.t >= ray.hit.t)
        return false;

    std::cout << "Successfull intersection:\n";

    ray.hit.t = result.t;
    ray.instIdx = primID;
    ray.hit.prim = result.brickID;
    // localVoxelPos will be encoded as x + y * bricksize + z * bricksize*bricksize NOT NEEDED
    //ray.hit.userInt32[0] = result.localVoxelPos.x + (result.localVoxelPos.y * BRICKSIZE) + (result.localVoxelPos.z * BRICKSIZE * BRICKSIZE);
    ray.hit.userInt32[0] = result.voxelPos.x;
    ray.hit.userInt32[1] = result.voxelPos.y;
    ray.hit.userInt32[2] = result.voxelPos.z;

    return true;
}



VoxelWorld::VoxelWorld()
{
    g_volumes = &voxelVolumes;

    voxelBVH = std::make_unique<tinybvh::BVH>();
    sphereBVH = std::make_unique<tinybvh::BVH>();
    lightAllocator = std::make_unique<GPUAllocator>(sizeof(Light), lights.size());
    lightAllocation = lightAllocator->Allocate(lights.size(), lights.data());
    voxelBVHAllocator = std::make_unique<GPUAllocator>(sizeof(tinybvh::BVH::BVHNode));
    voxelBVHPrimIndexAllocator = std::make_unique<GPUAllocator>(sizeof(uint));


    constexpr std::size_t brickCount = 200 / sizeof(Brick);
    voxelAllocator = std::make_unique<GPUAllocator>(sizeof(Brick), brickCount);
    brickIndexAllocator = std::make_unique<GPUAllocator>(sizeof(uint), brickCount);
    voxelVolumeAllocator = std::make_unique<GPUAllocator>(sizeof(VoxelVolume::GPUData), 8000);
    materialAllocator = std::make_unique<GPUAllocator>(sizeof(Material), 257 * 4);

    physicsWorld = std::make_unique<TIPHYS::PhysicsWorld>();

    voxelBVH->customIntersect  = &CustomIntersect;


    GenerateSpheres();
}

VoxelWorld::~VoxelWorld()
{// unique ptr moment.
}

void VoxelWorld::GenerateSpheres()
{
    //------------------------------------------------------------
    //                  SPHERE GENERATION :P
    //------------------------------------------------------------
    constexpr int sphereCount{ 1000 };

    

    spheres.resize(sphereCount);
    std::mt19937 rng{};
    std::uniform_real_distribution<float> randPosDist(-300.f, 300.f);
    std::uniform_real_distribution<float> randRadiusDist(0.2f, 5.f);
    std::uniform_int_distribution<int> randMaterial(1, 255);
    for (int i{ 0 }; i < sphereCount; i++) {
        Sphere s;
        s.materialID = 2;// randMaterial(rng);
        s.pos = glm::vec3(randPosDist(rng), randPosDist(rng), randPosDist(rng));
        s.radius = randRadiusDist(rng);
        spheres[i] = s;
    }
    sphereAllocator = std::make_unique<GPUAllocator>(sizeof(Sphere), sphereCount);
    sphereAllocation = sphereAllocator->Allocate(sphereCount, spheres.data());

    if constexpr(sphereCount > 0) {
        std::vector<tinybvh::bvhvec4> aabbs;
        for (int i{ 0 }; i < sphereCount; i++) {
            aabbs.emplace_back(tinybvh::bvhvec4(spheres[i].pos.x - spheres[i].radius, spheres[i].pos.y - spheres[i].radius, spheres[i].pos.z - spheres[i].radius, 0.f));
            aabbs.emplace_back(tinybvh::bvhvec4(spheres[i].pos.x + spheres[i].radius, spheres[i].pos.y + spheres[i].radius, spheres[i].pos.z + spheres[i].radius, 0.f));
        }
        sphereBVH->BuildAABB(aabbs.data(), sphereCount);
        sphereBVH->SplitLeafs(1);
    }

    sphereBVHAllocator = std::make_unique<GPUAllocator>(sizeof(tinybvh::BVH::BVHNode), sphereBVH->usedNodes);
    sphereBVHAllocation = sphereBVHAllocator->Allocate(sphereBVH->usedNodes, sphereBVH->bvhNode);

    sphereBVHPrimIndexAllocator = std::make_unique<GPUAllocator>(sizeof(uint), sphereCount);
    sphereBVHPrimIndexAllocation = sphereBVHPrimIndexAllocator->Allocate(sphereCount, sphereBVH->primIdx);
}




void VoxelWorld::LoadScene(const Scene& scene) {

    // clear volumes
    voxelVolumes.clear();
    voxelVolumeAllocations.clear();
    // always 1 empty material.
    materials.resize(1);
    
    // reset allocations
    voxelBVHAllocation.size = 0;
    voxelBVHPrimIndexAllocation.size = 0;
    currentBrickIndex = 0;
    totalVoxels = 0;
    filledVoxels = 0;


    // cleawr GPU buffers
    voxelAllocator->Reset();
    brickIndexAllocator->Reset();
    voxelVolumeAllocator->Reset();
    materialAllocator->Reset();
    voxelBVHAllocator->Reset();
    voxelBVHPrimIndexAllocator->Reset();


    // allocate 1 empty brick. // since brick index 0 is seen as an empty brick.
    voxelAllocator->Allocate(1, nullptr);

    // delete old physcis
    physicsWorld = std::make_unique<TIPHYS::PhysicsWorld>();

    
    for (const Scene::Model& m : scene.models) {
        LoadModel(m.fileLocation.c_str(), m.worldOrigin);
    }

    RebuildBVH(0.f);
}


void VoxelWorld::LoadModel(const char* fileName, glm::vec3 origin)
{
    MagicaParser::Scene scene = MagicaParser::ParseModel(fileName);
    
    //static_cast<int>(voxelVolumes.size())
    int vOffset = static_cast<int>(voxelVolumes.size());

    const uint materialStart = static_cast<uint>(materials.size() - 1);


    // help convert with claude because magica voxel stores stupidly.
    // MV Z-up → engine Y-up basis change (upper-left 3x3 of your C matrix).
    // C * v = (mv_x, mv_z, -mv_y)  — same mapping used on voxel data.
    // Defined once as mat3 so voxelSize doesn't need to touch it.
    static const glm::mat3 C3(
        glm::vec3(1, 0, 0),   // col0
        glm::vec3(0, 0, -1),   // col1
        glm::vec3(0, 1, 0)    // col2
    );
    static const glm::mat3 C3T = glm::transpose(C3); // C is orthogonal → inverse = transpose


    int j{ 0 };
    voxelVolumes.resize(vOffset + scene.instances.size());
    const int modelVolumeCount = static_cast<int>(scene.instances.size());
    std::vector<std::vector<Voxel>> volumesVoxelData(modelVolumeCount);
    for (auto& instance : scene.instances) {

        voxelVolumes[vOffset + j] = std::make_unique<VoxelVolume>();
        auto& volume = *voxelVolumes[vOffset + j];
        auto& mesh = scene.meshes[instance.ID];

        glm::ivec3 dim = mesh.extent;

        std::swap(dim.y, dim.z);

        int dataSize = dim.x * dim.y * dim.z;
        totalVoxels += dataSize;

        std::vector<Voxel>& data = volumesVoxelData[j];
        data.resize(dataSize);
        for (int i{ 0 }; i < mesh.voxels.size(); i++) {
            u8 x = mesh.voxels[i].x;
            // WE ARE SWAPPING Y and Z here
            u8 y = mesh.voxels[i].z;
            u8 z = static_cast<u8>(dim.z-1) - mesh.voxels[i].y;
            data[x + y * dim.x + z * dim.x * dim.y].materialID = mesh.voxels[i].materialIndex;
            filledVoxels++;
        }

        const glm::vec3 pivot    = (C3 * instance.position) * voxelSize + origin;
        const glm::mat3 rotation = C3 * instance.rotation * C3T;
        
        volume.LoadData(pivot,rotation,glm::ivec3(dim),currentBrickIndex, data, materialStart, *voxelAllocator, physicsWorld.get());

        currentBrickIndex += static_cast<uint>(volume.GetBrickIndexes().size());
        brickIndexAllocator->Allocate(volume.GetBrickIndexes().size(), volume.GetBrickIndexes().data());

        j++;
    }
    voxelVolumeAllocations.push_back(voxelVolumeAllocator->Allocate(modelVolumeCount, nullptr));

    constexpr int materialCount = 256;
    // skip first one because its empty
    materials.resize(materials.size() + materialCount);
    for (uint i{ 0 }; i < materialCount; i++) {
        auto& sceneMat = scene.materials[i];
        
        assert(sceneMat.id <= materialCount);

        int matId = static_cast<int>(materialStart) + sceneMat.id;
        Material mat{};
        auto& palColor = scene.palette[sceneMat.id];
        glm::vec3 color = glm::vec3(static_cast<float>(palColor.r) / 255.f, static_cast<float>(palColor.g) / 255.f, static_cast<float>(palColor.b) / 255.f);
        mat.color = color;
        mat.type = static_cast<MaterialType>(sceneMat.type);


        mat.emit = sceneMat.emit;
        mat.flux = sceneMat.flux;
        mat.ior = sceneMat.ior;

        materials[matId] = mat;
    }

    materialAllocator->Allocate(materialCount, materials.data() + materialStart);
}



void VoxelWorld::RebuildBVH(float delta)
{
    static float counter{};
    counter += delta;
    // 1 AABB = 
    // 2x bvhvec4
    std::vector<tinybvh::bvhvec4> aabbs;
    for (auto& volume : voxelVolumes) {
        TIPHYS::Box* box = volume->GetPhysicsBody();
        if (box == nullptr) {
            std::cout << "RebuildBVH: invalid BOX\n";
            continue;
        }
        const TIPHYS::AABB& aabb = box->GetAABB();

        aabbs.emplace_back(tinybvh::bvhvec4(aabb.start.x,delta + aabb.start.y, aabb.start.z, 0.f));
        aabbs.emplace_back(tinybvh::bvhvec4(aabb.end.x  ,delta + aabb.end.y  , aabb.end.z  , 0.f));
    }
    voxelBVH->BuildAABB(aabbs.data(), static_cast<uint32_t>(aabbs.size() / 2));
    voxelBVH->SplitLeafs(1);
    //std::cout << "Bvh build took" << t.elapsed() << '\n';
    

    GPUAllocation nodeAlloc{  0 , voxelBVH->usedNodes };
    GPUAllocation indexAlloc{ 0 , voxelVolumes.size() };

    voxelBVHAllocator->Upload(nodeAlloc, voxelBVH->bvhNode);
    voxelBVHPrimIndexAllocator->Upload(indexAlloc, voxelBVH->primIdx);
}


glm::vec3 ToVec3(const tinybvh::bvhvec4& v) {
    return glm::vec3(v.x, v.y, v.z);
}



void VoxelWorld::Update(float delta)
{
    bool displayBVH{ false };

    TryAddVolumes();

    if (physicsUpdate) {
        static float timeSinceStep{};
        timeSinceStep += delta;

        constexpr float physicsStepTime = 1.f / 60.f;
        while (timeSinceStep > physicsStepTime) {
            physicsWorld->Step(physicsStepTime);
            timeSinceStep -= physicsStepTime;
        }
    }
    
    if (physicsUpdate && voxelVolumes.size()) {
        RebuildBVH(delta);
    }


    GPUAllocation matUpload{ 0,materials.size() };
    materialAllocator->Upload(matUpload, materials.data());


    lightAllocator->Upload(lightAllocation, lights.data());
    lightAllocator->Bind(1);
    voxelBVHAllocator->Bind(2);
    voxelAllocator->Bind(3);
    for (int i{ 0 }; i < voxelVolumes.size(); i++) {
        GPUAllocation alloc;
        alloc.size = 1;
        alloc.offset = i;
        voxelVolumeAllocator->Upload(alloc, voxelVolumes[i]->GetGPUData());
    }
    voxelVolumeAllocator->Bind(4);
    materialAllocator->Bind(5);
    brickIndexAllocator->Bind(6);
    voxelBVHPrimIndexAllocator->Bind(7);
    //bitVoxelAllocator->Bind(13);
    sphereAllocator->Bind(13);
    sphereBVHAllocator->Bind(14);
    sphereBVHPrimIndexAllocator->Bind(15);

    int bvhDisplayID{ 0 };

    // voxelBVH bounding box
    if (displayBVH)
        Engine::Instance().debug->DrawAABB(AABB{
        //glm::vec3(voxelBVH->tlasNode[bvhDisplayID].aabbmin),glm::vec3(voxelBVH->tlasNode[bvhDisplayID].aabbmax) },
        ToVec3(voxelBVH->bvhNode[bvhDisplayID].aabbMin),ToVec3(voxelBVH->bvhNode[bvhDisplayID].aabbMax) },
        glm::vec3(1.f, 0.f, 0.f), 0.001f);

    for (Light& light : lights)
        Engine::Instance().debug->DrawAABB(AABB{
            light.position - 0.5f,light.position + 0.5f },
            glm::vec3(0.f, 1.f, 0.f), 0.001f);

}

tinybvh::bvhvec3 ToVec3(const glm::vec3& v) {
    return tinybvh::bvhvec3(v.x, v.y, v.z);
}

void VoxelWorld::Hit(const glm::vec3& origin, const glm::vec3& dir, float length)
{
    if (fracturingVolume)
        return;

    TraceResult hit = Trace(origin,dir,length);

    // do this on separate thread.
    if (hit.volume) {
        fracturingVolume = hit.volume;

        auto fracture = [](VoxelWorld* world, TraceResult hit, glm::vec3 dir) {
            world->volumesToAdd = hit.volume->Fracture(hit.voxelPos, dir);
            world->fracturingDone.store(true);
            };

        fractureThread = std::thread(fracture,this, hit,dir);
    }
}

void VoxelWorld::TryAddVolumes()
{
    if (!fracturingDone.load())
        return;

    if (volumesToAdd) {
        for (VoxelVolume::SplitVolume& split : *volumesToAdd) {
            voxelVolumes.emplace_back(std::make_unique<VoxelVolume>());
            VoxelVolume& volume = *voxelVolumes.back();

            volume.LoadData(split.pivot, fracturingVolume->GetRotation(), split.voxelDims, currentBrickIndex, split.voxels, 0, *voxelAllocator, physicsWorld.get(), false);
            currentBrickIndex += static_cast<uint>(volume.GetBrickIndexes().size());
            brickIndexAllocator->Allocate(volume.GetBrickIndexes().size(), volume.GetBrickIndexes().data());
            voxelVolumeAllocations.push_back(voxelVolumeAllocator->Allocate(1, nullptr));
        }
    }

    auto& filledBricks = fracturingVolume->GetFilledBricks();
    GPUAllocation alloc;
    alloc.offset = fracturingVolume->GetFilledBrickStartIndex();
    alloc.size = filledBricks.size();
    voxelAllocator->Upload(alloc, filledBricks.data());


    fracturingDone.store(false);
    fracturingVolume = nullptr;
    
    if(fractureThread.joinable())
        fractureThread.join();


    if (!physicsUpdate)
        RebuildBVH(0.f);

}

VoxelWorld::TraceResult VoxelWorld::Trace(const glm::vec3& origin, const glm::vec3& dir, float length)
{
    if (voxelVolumes.size() > 0) {
        // ray setup
        tinybvh::Ray ray;
        ray.D = ToVec3(glm::normalize(dir));
        ray.O = ToVec3(origin);
        for (int i{ 0 }; i < 3; i++) {
            if (isnan(ray.D[i]))
                ray.D[i] = 0.000001f;
            if (isnan(ray.O[i]))
                ray.O[i] = 0.000001f;
        }
        ray.rD = 1.f / ray.D;
        ray.hit.t = length;
        ray.hit.inst = UINT_MAX;


        voxelBVH->Intersect(ray);
        
        if (ray.hit.inst < voxelVolumes.size()) {
            std::cout << "Hit:\t" << ray.hit.inst << " Brick: " << ray.hit.prim  << '\n';
            VoxelVolume& volume = *voxelVolumes[ray.hit.inst];

            glm::uvec3 voxelPos{};
            voxelPos.x = ray.hit.userInt32[0];
            voxelPos.y = ray.hit.userInt32[1];
            voxelPos.z = ray.hit.userInt32[2];
            
            if (false) {
                Brick filledBrick{};
                for (auto& val : filledBrick.voxels)
                    val = 3;
                GPUAllocation alloc;
                alloc.offset = ray.hit.prim;
                alloc.size = 1;
                voxelAllocator->Upload(alloc, &filledBrick);
                Engine::Instance().debug->DrawBox(origin + glm::normalize(dir) * ray.hit.t, glm::vec3(1.f), glm::vec3(1.f), 5.f);
            }
            return { &volume,voxelPos };
        }
    }
    return { nullptr };
}
