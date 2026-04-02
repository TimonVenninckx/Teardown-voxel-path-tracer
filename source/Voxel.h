#pragma once
#include "engine/Common.h"

struct Voxel {
    unsigned short materialID;
};

constexpr uint BRICKSIZE{ 8 };
struct Brick {
    unsigned short voxels[BRICKSIZE * BRICKSIZE * BRICKSIZE]{ 0 };
};


// 8*8*8 = 512 / 32 = 16
//struct BrickMask {
//    void SetVoxel(uint id) {
//        uint arrayIndex = id >> 5;
//        uint maskWith1BitSet =  1 << (id & 31);
//        voxels[arrayIndex] |= maskWith1BitSet;
//    }
//    uint voxels[16]{ 0 };
//};