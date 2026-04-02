#include "VoxelVolume.h"
#include "VoxelWorld.h"
#include "TIPHYS/PhysicsWorld.h"
#include "TIPHYS/Body.h"
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


constexpr uint MAXSTEPS{ 24 };
const float& voxelSize = VoxelWorld::voxelSize;




void VoxelVolume::LoadData(const glm::vec3& pivot,const glm::mat3& rotation,const glm::uvec3& voxelDims, uint currentBrickIndex, 
    const std::vector<Voxel>& voxels, uint materialStart, GPUAllocator& brickAllocator, TIPHYS::PhysicsWorld* world, bool staticBody)
{
    const glm::vec3 localPivot = glm::vec3(
        voxelDims.x / 2,
        voxelDims.y / 2,
        (voxelDims.z) - (voxelDims.z / 2)  // corrects for the z-flip
    ) * VoxelWorld::voxelSize;
    const glm::vec3 corner = pivot - rotation * localPivot;


    gpuData.model = glm::mat4(rotation);
    gpuData.model[3] = glm::vec4(corner, 1.0f);
    gpuData.invModel = glm::inverse(gpuData.model);
    gpuData.voxelDims = voxelDims;
    gpuData.brickIndexStart = currentBrickIndex;

    filledBrickStartIndex = 0;
    
    const glm::vec3 halfExtents = 0.5f * glm::vec3(voxelDims) * VoxelWorld::voxelSize;

    TIPHYS::Box box;
    box.position = corner + rotation * halfExtents;
    box.halfExtents = halfExtents;
    box.rotation = rotation;
    box.mass = static_cast<float>(voxelDims.x * voxelDims.y * voxelDims.z);

    if(staticBody)
        physicsHandle = world->AddStaticBox(box);
    else
        physicsHandle = world->AddBox(box);

    brickBounds = (voxelDims + (BRICKSIZE - 1)) / BRICKSIZE;

    for (uint brickz{ 0 }; brickz < brickBounds.z; brickz++) {
    for (uint bricky{ 0 }; bricky < brickBounds.y; bricky++) {
    for (uint brickx{ 0 }; brickx < brickBounds.x; brickx++) {
        Brick brick{ 0 };
        //BrickMask brickMask{ 0 };
        bool voxelInsideBrick{ false };

        const glm::uvec3 insideBrickBounds{ BRICKSIZE };

        for (uint voxz{ 0 }; voxz < insideBrickBounds.z; voxz++) {
        for (uint voxy{ 0 }; voxy < insideBrickBounds.y; voxy++) {
        for (uint voxx{ 0 }; voxx < insideBrickBounds.x; voxx++) {
            uint x = voxx + brickx * BRICKSIZE;
            uint y = voxy + bricky * BRICKSIZE;
            uint z = voxz + brickz * BRICKSIZE;
            if (x >= voxelDims.x
                || y >= voxelDims.y
                || z >= voxelDims.z)
                continue;

            uint voxelId = voxels[x + y * voxelDims.x + z * voxelDims.x * voxelDims.y].materialID;
            if (voxelId > 0)
                voxelId += materialStart;

            if (voxelId) {
                uint index = voxx + voxy * BRICKSIZE + voxz * BRICKSIZE * BRICKSIZE;
                brick.voxels[index] = static_cast<unsigned short>(voxelId);
                voxelInsideBrick = true;
            }
        }
        }
        }
        if (voxelInsideBrick) {
            // gpu memory allocation
            GPUAllocation alloc = brickAllocator.Allocate(1, &brick);
            if (filledBrickStartIndex == 0)
                filledBrickStartIndex = static_cast<uint>(alloc.offset);
            brickIndices.emplace_back(static_cast<int>(alloc.offset));
            filledBricks.emplace_back(brick);
        }
        else { // empty brick
            brickIndices.emplace_back(0);
        }
    }
    }
    }
}



void VoxelVolume::SetPosition(const glm::vec3& position) {
    TIPHYS::Box* box = GetPhysicsBody();
    assert(box);
    box->position = position;
    box->velocity = glm::vec3(0.f);
    box->angularVelocity = glm::vec3(0.f);
    box->isDirty = true;
    box->GetAABB();// to rebuild BVH
}

glm::vec3 VoxelVolume::GetPosition() {
    TIPHYS::Box* box = GetPhysicsBody();
    assert(box);
    return box->position;
}

glm::mat3 VoxelVolume::GetRotation()const
{
    //TIPHYS::Box* box = GetPhysicsBody();
    //assert(box);
    return  glm::mat3(gpuData.model);
}



uint VoxelVolume::GetBrickIndex(const glm::uvec3& brickOffset) const{
    // first calculate index to the list by using x + y * size.x + z * size.x * size.y
    uint brickIndex = brickOffset.x + brickOffset.y * brickBounds.x + brickOffset.z * brickBounds.x * brickBounds.y;
    return brickIndices[brickIndex];
}

uint VoxelVolume::GetVoxel(const glm::uvec3& localVoxelPos) const
{
    glm::uvec3 brick = localVoxelPos / BRICKSIZE;
    uint brickIndex = GetBrickIndex(brick);
    if (brickIndex > 0) {
        uint localIndex = brickIndex - filledBrickStartIndex;


        glm::uvec3 posInBrick = localVoxelPos % BRICKSIZE;

        uint voxelIndex = posInBrick.x + posInBrick.y * BRICKSIZE + posInBrick.z * BRICKSIZE * BRICKSIZE;
        return filledBricks[localIndex].voxels[voxelIndex];
    }
    return 0;
}

bool VoxelVolume::SetVoxel(const glm::uvec3& localVoxelPos, uint newVoxel) {
    glm::uvec3 brick = localVoxelPos / BRICKSIZE;
    uint brickIndex = GetBrickIndex(brick);
    if (brickIndex > 0) {
        uint localIndex = brickIndex - filledBrickStartIndex;

        glm::uvec3 voxelPosInBrick = localVoxelPos % BRICKSIZE;
        uint voxelIndex = voxelPosInBrick.x + voxelPosInBrick.y * BRICKSIZE + voxelPosInBrick.z * BRICKSIZE * BRICKSIZE;
        filledBricks[localIndex].voxels[voxelIndex] = static_cast<unsigned short>(newVoxel);
        return true;
    }
    return false;
}

uint VoxelVolume::GetVoxel(uint brickIndex, const glm::uvec3& voxelPosInBrick) const{

    if (brickIndex > 0) {
        uint localIndex = brickIndex - filledBrickStartIndex;
        
        uint voxelIndex = voxelPosInBrick.x + voxelPosInBrick.y * BRICKSIZE + voxelPosInBrick.z * BRICKSIZE* BRICKSIZE;
        return filledBricks[localIndex].voxels[voxelIndex];
    }
    return 0;
}



bool VoxelVolume::SetVoxel(uint brickIndex, const glm::uvec3& voxelPosInBrick, uint newVoxel)
{
    if (brickIndex > 0) {
        uint localIndex = brickIndex - filledBrickStartIndex;

        uint voxelIndex = voxelPosInBrick.x + voxelPosInBrick.y * BRICKSIZE + voxelPosInBrick.z * BRICKSIZE* BRICKSIZE;
        filledBricks[localIndex].voxels[voxelIndex] = static_cast<unsigned short>(newVoxel);
        return true;
    }
    return false;
}


uint VoxelVolume::TraverseBrick(const glm::vec3& origin, const glm::vec3& dir, const glm::uvec3& brickCoord, float& t, glm::uvec3& voxelPos)const {

    // removing an epsilon
    float tEntry = t;
    glm::vec3 localPos = origin + dir * tEntry;

    glm::ivec3 vstep = glm::ivec3(glm::sign(dir));


    uint brickID = GetBrickIndex(brickCoord);


    glm::uvec3 brickBase = brickCoord * glm::uvec3(BRICKSIZE);
    voxelPos = glm::uvec3(glm::clamp(
        glm::uvec3(glm::uvec3(glm::floor(localPos / voxelSize)) - brickBase),
        glm::uvec3(0),
        glm::uvec3(BRICKSIZE - 1)
    ));

    glm::uvec3 globalVoxel = brickBase + voxelPos;

    glm::vec3 nextVoxelBoundary;
    nextVoxelBoundary.x = (vstep.x > 0) ?
        (static_cast<float>(globalVoxel.x) + 1.0f) * voxelSize : static_cast<float>(globalVoxel.x) * voxelSize;
    nextVoxelBoundary.y = (vstep.y > 0) ?
        (static_cast<float>(globalVoxel.y) + 1.0f) * voxelSize : static_cast<float>(globalVoxel.y) * voxelSize;
    nextVoxelBoundary.z = (vstep.z > 0) ?
        (static_cast<float>(globalVoxel.z) + 1.0f) * voxelSize : static_cast<float>(globalVoxel.z) * voxelSize;


    const glm::vec3 invdir = glm::vec3(1.0f) / glm::max(glm::abs(dir), glm::vec3(0.000001f)) * glm::sign(dir);
    glm::vec3 tMax = (nextVoxelBoundary - localPos) * invdir;
    glm::vec3 tDelta = voxelSize * glm::abs(invdir); 


    if (voxelPos.x >= BRICKSIZE
        || voxelPos.y >= BRICKSIZE
        || voxelPos.z >= BRICKSIZE)
        return 0;


    uint voxelID = GetVoxel(brickID, voxelPos);

    // start inside
    for (int n = 0; n < MAXSTEPS; n++) {
        // reading data
        if (voxelID > 0) {
            return voxelID;
        }
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            voxelPos.x += vstep.x;
            t = tEntry + tMax.x;
            tMax.x += tDelta.x;
            if (voxelPos.x >= BRICKSIZE)
                return 0;
        }
        else if (tMax.y < tMax.z) {
            voxelPos.y += vstep.y;
            t = tEntry + tMax.y;
            tMax.y += tDelta.y;
            if (voxelPos.y >= BRICKSIZE)
                return 0;
        }
        else {
            voxelPos.z += vstep.z;
            t = tEntry + tMax.z;
            tMax.z += tDelta.z;
            if (voxelPos.z >= BRICKSIZE)
                return 0;
        }
        voxelID = GetVoxel(brickID, voxelPos);
    }
    return 0;
}

float IntersectAABBFloat(glm::vec3 origin, glm::vec3 dir,float currentT, const glm::vec3 bmin, const glm::vec3 bmax) {
    glm::vec3 invDir = glm::vec3(1.0f) / glm::max(glm::abs(dir), glm::vec3(1e-6f)) * glm::sign(dir);

    float tx1 = (bmin.x - origin.x) * invDir.x;
    float tx2 = (bmax.x - origin.x) * invDir.x;
    float tmin = glm::min(tx1, tx2);
    float tmax = glm::max(tx1, tx2);
    float ty1 = (bmin.y - origin.y) * invDir.y;
    float ty2 = (bmax.y - origin.y) * invDir.y;
    tmin = glm::max(tmin, glm::min(ty1, ty2));
    tmax = glm::min(tmax, glm::max(ty1, ty2));
    float tz1 = (bmin.z - origin.z) * invDir.z;
    float tz2 = (bmax.z - origin.z) * invDir.z;
    tmin = glm::max(tmin, glm::min(tz1, tz2));
    tmax = glm::min(tmax, glm::max(tz1, tz2));

    if (tmax >= tmin && tmin < currentT && tmax > 0.f)
        return tmin;
    return 1e34f;
}


VoxelVolume::IntersectResult VoxelVolume::Intersect(const glm::vec3& inOrigin,const glm::vec3& inDir)
{
    TIPHYS::Box* box = GetPhysicsBody();
    if (!box) {
        return { 0,1e34f };
    }

    TIPHYS::AABB aabb = box->GetAABB();

    glm::vec3 curMin = glm::vec3(0.f);
    glm::vec3 curMax = glm::vec3(gpuData.voxelDims) * voxelSize;

    glm::vec3 origin =  gpuData.invModel * glm::vec4(inOrigin, 1.f);
    glm::vec3 dir    =  gpuData.invModel * glm::vec4(inDir   , 0.f);
    
    float t = IntersectAABBFloat(origin, dir, 1e34f, curMin, curMax);

    if (t >= 1e30f)
        return { 0,1e34f };

    t = glm::max(0.0f, t) + 0.0005f * voxelSize;
    // removing an epsilon
    float tEntry = t;
    const glm::vec3 perBrickSize = glm::vec3(voxelSize) * static_cast<float>(BRICKSIZE);

    glm::vec3 localPos = origin + dir * (tEntry);

    glm::uvec3 brick = glm::ivec3(glm::floor(localPos / perBrickSize));
    const glm::ivec3 bstep = glm::ivec3(glm::sign(dir));


    glm::vec3 nextBrickBoundary;
    nextBrickBoundary.x = (bstep.x > 0) ?
        (static_cast<float>(brick.x) + 1.0f) * perBrickSize.x : static_cast<float>(brick.x) * perBrickSize.x;
    nextBrickBoundary.y = (bstep.y > 0) ?
        (static_cast<float>(brick.y) + 1.0f) * perBrickSize.y : static_cast<float>(brick.y) * perBrickSize.y;
    nextBrickBoundary.z = (bstep.z > 0) ?
        (static_cast<float>(brick.z) + 1.0f) * perBrickSize.z : static_cast<float>(brick.z) * perBrickSize.z;


    const glm::vec3 invdir = glm::vec3(1.0f) / glm::max(glm::abs(dir), glm::vec3(0.000001f)) * glm::sign(dir);
    glm::vec3 tMax = (nextBrickBoundary - localPos) * invdir;
    glm::vec3 tDelta = perBrickSize * glm::abs(invdir);

    // should never happen really..
    if (brick.x >= brickBounds.x
        || brick.y >= brickBounds.y
        || brick.z >= brickBounds.z)
        return { 0,1e34f };

    // safety measure. no while loop
    uint maxSteps = brickBounds.x + brickBounds.y + brickBounds.z;
    for (uint n = 0; n < maxSteps; n++) {

        float tempT = t + 0.0001f * voxelSize;
        uint brickID = GetBrickIndex(brick);
        if (brickID) {
            glm::uvec3 localVoxelPos;
            if (TraverseBrick(origin, dir, brick, tempT, localVoxelPos)) {
                return { brickID ,tempT, brick * BRICKSIZE + localVoxelPos};
            }
        }

        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            brick.x += bstep.x;
            t = tEntry + tMax.x;
            tMax.x += tDelta.x;
            if (brick.x >= brickBounds.x)
                break;
        }
        else if (tMax.y < tMax.z) {
            brick.y += bstep.y;
            t = tEntry + tMax.y;
            tMax.y += tDelta.y;
            if (brick.y >= brickBounds.y)
                break;
        }
        else {
            brick.z += bstep.z;
            t = tEntry + tMax.z;
            tMax.z += tDelta.z;
            if (brick.z >= brickBounds.z)
                break;
        }
    }
    return { 0,1e34f };
}

std::unique_ptr<std::vector<VoxelVolume::SplitVolume>> VoxelVolume::Fracture(const glm::uvec3& voxelImpactPos, const glm::vec3& impactDir)
{
    const uint maxRadius = 40;
    const glm::uvec3 startPos = glm::max(glm::ivec3(voxelImpactPos) - glm::ivec3(maxRadius), glm::ivec3(0));
    const glm::uvec3 endPos = glm::min(glm::ivec3(voxelImpactPos) + glm::ivec3(maxRadius), glm::ivec3(gpuData.voxelDims));

    const glm::uvec3 boxSize = endPos - startPos;

    static std::mt19937 rand;

    std::uniform_real_distribution<float> randX(static_cast<float>(startPos.x), static_cast<float>(endPos.x));
    std::uniform_real_distribution<float> randY(static_cast<float>(startPos.y), static_cast<float>(endPos.y));
    std::uniform_real_distribution<float> randZ(static_cast<float>(startPos.z), static_cast<float>(endPos.z));
    std::uniform_real_distribution<float> randNudge(-0.2f,0.2f);
    std::uniform_real_distribution<float> randVoronoiPointLength(40.f, 60.f);
    std::uniform_int_distribution<int>    randVoronoiPointCount(4, 7);

    std::vector<glm::vec3> voronoiPoints{};
    glm::vec3 voronoiUp = glm::vec3(0.f,1.f,0.f);
    if (abs(glm::dot(voronoiUp, impactDir) > 0.9f))
        voronoiUp = glm::vec3(1.f, 0.f, 0.f);
    const glm::vec3 voronoiRight = glm::cross(impactDir, voronoiUp);
    voronoiUp = glm::cross(impactDir, voronoiRight);

    voronoiPoints.emplace_back(voxelImpactPos);
    int voronoiPointCount = randVoronoiPointCount(rand);
    //voronoiPoints.emplace_back(voxelImpactPos);
    for (int i{ 0 }; i < voronoiPointCount; i++) {
        glm::vec3 point;
        if (false) {
            point = glm::vec3(randX(rand), randY(rand), randZ(rand));
        }
        else {
            float s = randNudge(rand) + i * (glm::pi<float>() * 2.f / static_cast<float>(voronoiPointCount));
            point = sin(s) * voronoiRight
                + cos(s) * voronoiUp
                + randNudge(rand) * impactDir * 4.f;
            
            point *= randVoronoiPointLength(rand);
            point += voxelImpactPos;

            point = glm::clamp(point, glm::vec3(startPos), glm::vec3(endPos));
        }
        voronoiPoints.emplace_back(point);
    }

    // local list of voxels with their VoronoiPoint ID
    std::vector<uint> voxels(boxSize.x * boxSize.y * boxSize.z);
    for (uint x{ 0 }; x < boxSize.x; x++) {
        for (uint y{ 0 }; y < boxSize.y; y++) {
            for (uint z{ 0 }; z < boxSize.z; z++) {

                int nearestSeed = -1;
                float closestDistanceSqr = FLT_MAX;
                glm::vec3 pos = glm::vec3(x, y, z) + glm::vec3(startPos);
                for (int i{ 0 }; i < voronoiPoints.size(); i++) {
                    glm::vec3 v = voronoiPoints[i] - pos;
                    float dSqr = glm::dot(v, v);
                    if (dSqr < closestDistanceSqr) {
                        closestDistanceSqr = dSqr;
                        nearestSeed = i;
                    }
                }
                voxels[x + y * boxSize.x + (z * boxSize.x * boxSize.y)] = nearestSeed;
            }
        }
    }

    auto SameVoronoi = [&](uint curVoronoi, const glm::ivec3& pos)->bool {

        if (glm::any(glm::lessThan(pos, glm::ivec3(0))) ||
            glm::any(glm::greaterThanEqual(pos, glm::ivec3(boxSize))))
            return true;

        return voxels[pos.x + boxSize.x * pos.y + boxSize.x * boxSize.y * pos.z] == curVoronoi;
        };


    std::vector<glm::ivec3> destroyedVoxelPos{};
    for (uint x{ 0 }; x < boxSize.x; x++) {
        for (uint y{ 0 }; y < boxSize.y; y++) {
            for (uint z{ 0 }; z < boxSize.z; z++) {
                glm::ivec3 localPos = glm::uvec3{ x,y,z };
                
                uint curVoronoi = voxels[x + y * boxSize.x + (z * boxSize.x * boxSize.y)];
                if (   !SameVoronoi(curVoronoi, localPos + glm::ivec3( 1, 0, 0))
                    || !SameVoronoi(curVoronoi, localPos + glm::ivec3(-1, 0, 0))
                    || !SameVoronoi(curVoronoi, localPos + glm::ivec3( 0, 1, 0))
                    || !SameVoronoi(curVoronoi, localPos + glm::ivec3( 0,-1, 0))
                    || !SameVoronoi(curVoronoi, localPos + glm::ivec3( 0, 0, 1))
                    || !SameVoronoi(curVoronoi, localPos + glm::ivec3( 0, 0,-1))
                    ) {
                    if(SetVoxel(startPos + glm::uvec3(localPos), 0))
                        destroyedVoxelPos.emplace_back(startPos + glm::uvec3(localPos));
                }
            }
        }
    }

    constexpr glm::ivec3 neighbours[6] {
        glm::ivec3(1, 0, 0),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 1, 0),
        glm::ivec3(0,-1, 0),
        glm::ivec3(0, 0, 1),
        glm::ivec3(0, 0,-1)
    };
    std::unordered_set<glm::ivec3> borderSet{};

    for (const glm::ivec3& p : destroyedVoxelPos) {
        for (int i{ 0 }; i < 6; i++) {
            glm::ivec3 vox = p + neighbours[i];
            if (glm::any(glm::lessThan(vox, glm::ivec3(0))) ||
                glm::any(glm::greaterThanEqual(vox, glm::ivec3(gpuData.voxelDims))))
                continue;

            if (GetVoxel(vox)) {
                borderSet.insert(vox);
            }
        }
    }
    if (borderSet.empty())
        return nullptr;



    auto BFS = [&neighbours, this](glm::ivec3 startPos, std::unordered_map<glm::ivec3,unsigned short>& visited) {
        
        std::queue<glm::ivec3> pointsToVisit{};
        pointsToVisit.emplace(startPos);
        {
            unsigned short voxel = static_cast<unsigned short>(GetVoxel(startPos));
            visited.emplace(startPos, voxel);
        }
        while (!pointsToVisit.empty()) {
            const glm::ivec3 cur = pointsToVisit.front();
            pointsToVisit.pop();

            for (const glm::ivec3& n : neighbours) {
                const glm::ivec3 p = cur + n;
                if (glm::any(glm::lessThan(p, glm::ivec3(0))) ||
                    glm::any(glm::greaterThanEqual(p, glm::ivec3(gpuData.voxelDims))))
                    continue;

                if (visited.find(p) == visited.end()) {
                    if (unsigned short voxel = static_cast<unsigned short>(GetVoxel(p))) {
                        pointsToVisit.push(p);
                        visited.emplace( p, voxel );
                    }
                }
            }
        }
        };

    std::unordered_map<glm::ivec3, unsigned short> visited;
    std::unique_ptr<std::vector<SplitVolume>> splitVolumes = std::make_unique<std::vector<SplitVolume>>();



    int largestSize = 0;
    int largestVolume = -1;
    glm::ivec3 largestVolumeStartPos{};
    // find islands
    for (const glm::ivec3& seed : borderSet) {
        if (visited.find(seed) != visited.end())
            continue;

        std::unordered_map<glm::ivec3, unsigned short> islandVoxels;
        BFS(seed, islandVoxels);

        splitVolumes->push_back(SplitVolume());
        SplitVolume& volume = splitVolumes->back();
        
        glm::ivec3 min = glm::ivec3(INT_MAX);
        glm::ivec3 max = glm::ivec3(INT_MIN);
        for (auto& [key,voxel] : islandVoxels) {
            min = glm::min(min, key);
            max = glm::max(max, key);
        }
        max += 1; // because 0 index start

        glm::ivec3 voxelCenter = min + (max - min) / 2;
        
        glm::vec3 localOffset = glm::vec3(voxelCenter) - glm::vec3(gpuData.voxelDims) * 0.5f;
        volume.pivot = GetPosition() + GetRotation() * (localOffset * voxelSize);
        
        volume.voxelDims = max - min;
        volume.voxels.resize(volume.voxelDims.x * volume.voxelDims.y * volume.voxelDims.z);
        {
            int size = volume.voxelDims.x * volume.voxelDims.y * volume.voxelDims.z;
            if (size > largestSize) {
                largestSize = size;
                largestVolume = static_cast<int>(splitVolumes->size()) - 1;
                largestVolumeStartPos = min;
            }
        }
        
        for (auto& [key, voxel] : islandVoxels) {
            glm::uvec3 pos = key - min;
            Voxel v;
            v.materialID = voxel;
            volume.voxels[pos.x + (pos.y * volume.voxelDims.x) + (pos.z * volume.voxelDims.x * volume.voxelDims.y)] = v;
        }
        visited.merge(islandVoxels);
    }

    if (splitVolumes->size() <= 1)
        return nullptr;

    // remove biggest from list;
    const auto& largestVol = (*splitVolumes)[largestVolume];
    
    std::memset(filledBricks.data(), 0, filledBricks.size() * sizeof(Brick));
    for (int i{ 0 }; i < largestVol.voxels.size(); i++) {

        glm::ivec3 voxelPos = largestVolumeStartPos;
        voxelPos += glm::ivec3(i % largestVol.voxelDims.x, (i / largestVol.voxelDims.x) % largestVol.voxelDims.y, i / (largestVol.voxelDims.x * largestVol.voxelDims.y));
        SetVoxel(voxelPos, largestVol.voxels[i].materialID);
    }
    splitVolumes->erase(splitVolumes->begin() + largestVolume);
    

    return std::move(splitVolumes);
}

void VoxelVolume::SetPhysicsStatic(bool isStatic)
{
    TIPHYS::Box* box = GetPhysicsBody();
    if (!box) {
        printf("VoxeVolume::SetPhysicsStatic|| failed\n");
        return;
    }
    TIPHYS::Box& b = *box;
    if (isStatic) {
        b.mass = FLT_MAX;
        b.invMass = 0.f;

        b.invInertiaLocal = glm::mat3(0.f);
        b.invInertiaWorld = glm::mat3(0.f);
    }
    else {
        b.mass = 10.f;
        b.invMass = 1.f / b.mass;

        const float hx2 = b.halfExtents.x * b.halfExtents.x;
        const float hy2 = b.halfExtents.y * b.halfExtents.y;
        const float hz2 = b.halfExtents.z * b.halfExtents.z;

        float ix = (1.f / 3.f) * b.mass * (hy2 + hz2);
        float iy = (1.f / 3.f) * b.mass * (hx2 + hz2);
        float iz = (1.f / 3.f) * b.mass * (hx2 + hy2);

        b.invInertiaLocal = glm::inverse(
            glm::mat3(ix, 0.f, 0.f,
                0.f, iy, 0.f,
                0.f, 0.f, iz
            ));
        b.invInertiaWorld = b.invInertiaLocal;
    }
}

const VoxelVolume::GPUData* VoxelVolume::GetGPUData()
{
    const glm::vec3 corner = physicsHandle->position - physicsHandle->rotation * physicsHandle->halfExtents;
    gpuData.model = glm::mat4(physicsHandle->rotation);
    gpuData.model[3] = glm::vec4(corner, 1.0f);
    gpuData.invModel = glm::inverse(gpuData.model);

    return &gpuData;
}

