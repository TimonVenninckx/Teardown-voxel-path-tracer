#pragma once

#include "engine/Common.h"
#include <vector>
#include <glm/vec3.hpp>

// wrote my own BVH
// not used anymore, currently use tinybvh.
// https://jacco.ompf2.com/2022/06/15/how-to-build-a-bvh-part-9b-massive/

//class VoxelVolume;
//
//
//struct TLASNode {
//    glm::vec3 aabbmin{};
//    uint leftright{}; // 2x 16 bits
//    glm::vec3 aabbmax{};
//    uint blas{};
//    bool IsLeaf() { return leftright == 0; }
//};
//
//
//class TLAS
//{
//public:
//    TLAS(std::vector<VoxelVolume>& volumes);
//    void Build();
//
//    int FindBestMatch(std::vector<int>& list, int n, int a);
//    
//    uint nodesUsed{};
//    uint blasCount{};
//    std::vector<TLASNode> tlasNode{};
//    std::vector<VoxelVolume>& blas;
//private:
//
//};

