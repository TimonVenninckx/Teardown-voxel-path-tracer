#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "engine/common.h"

#include "engine/AABB.h"
#include "TIPHYS/Body.h"
#include "Voxel.h"

#include <vector>
#include <memory>

class GPUAllocator;

class VoxelVolume {
public:
    VoxelVolume() {};
    ~VoxelVolume() {};

    struct SplitVolume {
        glm::vec3 pivot{};
        std::vector<Voxel> voxels{};
        glm::uvec3 voxelDims{};
    };

    struct GPUData {
        glm::mat4 model{1.f};
        glm::mat4 invModel{1.f};
        // amount of voxels in each direction.
        glm::uvec3 voxelDims{};
        uint brickIndexStart{ 0 };// go into array
    };

    void LoadData(const glm::vec3& pivot, const glm::mat3& rotation, const glm::uvec3& voxelDims, uint currentBrickIndex, 
        const std::vector<Voxel>& voxels, uint materialStart, GPUAllocator& brickAllocator, TIPHYS::PhysicsWorld* world, bool staticBody = true);

    void SetPosition(const glm::vec3& position);
    // can't be made const.
    glm::vec3 GetPosition();
    glm::mat3 GetRotation() const;
    
    const std::vector<uint>& GetBrickIndexes()const {
        return brickIndices;
    }
    const std::vector<Brick>& GetFilledBricks()const {
        return filledBricks;
    }

    uint GetFilledBrickStartIndex()const {
        return filledBrickStartIndex;
    }

    struct IntersectResult {
        uint brickID{};
        float t{};
        glm::uvec3 voxelPos{};
    };
    IntersectResult Intersect(const glm::vec3& origin,const glm::vec3& dir);



    std::unique_ptr<std::vector<SplitVolume>> Fracture(const glm::uvec3& voxelImpactPos, const glm::vec3& impactDir);
    void SetPhysicsStatic(bool isStatic);

    const GPUData* GetGPUData();
    TIPHYS::Box* GetPhysicsBody() { return physicsHandle.Get(); }

private:

    uint TraverseBrick(const glm::vec3& origin,const glm::vec3& dir, const glm::uvec3& brickCoord, float& t, glm::uvec3& voxelPos)const;
    uint GetBrickIndex(const glm::uvec3& brickOffset) const;
    
    uint GetVoxel(const glm::uvec3& localVoxelPos) const;
    bool SetVoxel(const glm::uvec3& localVoxelPos, uint newVoxel);

    uint GetVoxel(uint brickID, const glm::uvec3& voxelPosInBrick)const;
    bool SetVoxel(uint brickID,const glm::uvec3& voxelPosInBrick, uint newVoxel);


    TIPHYS::PhysicsBodyHandle physicsHandle{};

    GPUData gpuData{};

    // where the first filled brick is located on gpu memory
    // it is also used to derrive the correct filledbricks index.
    uint filledBrickStartIndex{};
    glm::uvec3 brickBounds{};
    std::vector<uint> brickIndices{};
    std::vector<Brick> filledBricks;

};