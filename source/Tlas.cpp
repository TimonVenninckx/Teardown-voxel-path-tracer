#include "Tlas.h"
#include "VoxelVolume.h"

//TLAS::TLAS(std::vector<VoxelVolume>& volumes)
//    : blas{volumes}
//{
//    blasCount = static_cast<uint>(volumes.size());
//    tlasNode.resize(blasCount * 2);
//    nodesUsed = 2;
//}
//
//void TLAS::Build()
//{
//    // asign a node to each BLAS
//    std::vector<int> nodeIdx(blasCount * 2);
//    // skip Node 0 because of memory in registers
//    nodesUsed = 1;
//
//    int nodeIndices = blasCount;
//
//    // asign nodes
//    for (uint i = 0; i < blasCount; i++) {
//        nodeIdx[i] = nodesUsed;
//        tlasNode[nodesUsed].aabbmin = glm::vec3(blas[i].aabbmin);
//        tlasNode[nodesUsed].aabbmax = glm::vec3(blas[i].aabbmax);
//        tlasNode[nodesUsed].blas = i;
//        // its a leaf
//        tlasNode[nodesUsed++].leftright = 0; 
//    }
//
//    int A{ 0 };
//    int B = FindBestMatch(nodeIdx, nodeIndices, A);
//    while (nodeIndices > 1) {
//        int C = FindBestMatch(nodeIdx, nodeIndices, B);
//        if (A == C) {
//            int nodeIdxA = nodeIdx[A];
//            int nodeIdxB = nodeIdx[B];
//
//            TLASNode& nodeA = tlasNode[nodeIdxA];
//            TLASNode& nodeB = tlasNode[nodeIdxB];
//            TLASNode& newNode = tlasNode[nodesUsed];
//            // pack 16bit integers into 32bit
//            newNode.leftright = nodeIdxA + (nodeIdxB << 16);
//            
//            newNode.aabbmin = glm::vec3(
//                std::min(nodeA.aabbmin.x, nodeB.aabbmin.x),
//                std::min(nodeA.aabbmin.y, nodeB.aabbmin.y),
//                std::min(nodeA.aabbmin.z, nodeB.aabbmin.z)
//            );
//            newNode.aabbmax = glm::vec3(
//                std::max(nodeA.aabbmax.x, nodeB.aabbmax.x),
//                std::max(nodeA.aabbmax.y, nodeB.aabbmax.y),
//                std::max(nodeA.aabbmax.z, nodeB.aabbmax.z)
//            );
//
//            // A starts from blascount
//            nodeIdx[A] = nodesUsed++;
//            // goes downward
//            nodeIdx[B] = nodeIdx[nodeIndices - 1];
//            B = FindBestMatch(nodeIdx, --nodeIndices, A);
//        }
//        else {
//            A = B;
//            B = C;
//        }
//    }
//    tlasNode[0] = tlasNode[nodeIdx[A]];
//}
//
//
//
//
//int TLAS::FindBestMatch(std::vector<int>& list, int N, int A)
//{
//    float smallest = 1e30f;
//    int bestB = -1;
//    for (int B = 0; B < N; B++) {
//        if (B != A) {
//            glm::vec3 bmax = glm::max(tlasNode[list[A]].aabbmax, tlasNode[list[B]].aabbmax);
//            glm::vec3 bmin = glm::min(tlasNode[list[A]].aabbmin, tlasNode[list[B]].aabbmin);
//            glm::vec3 e = bmax - bmin;
//            float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
//            if (surfaceArea < smallest) {
//                smallest = surfaceArea;
//                bestB = B;
//            }
//        }
//    }
//    return bestB;
//}