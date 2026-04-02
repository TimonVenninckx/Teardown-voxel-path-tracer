#pragma once
#include <glm/vec3.hpp>




struct AABB {
    glm::vec3 start{};
    glm::vec3 end{};

    // ok this is cursed
    /*bool Overlap(const AABB& b)const
    {
        if (   this->end.x <= b.start.x
            || this->start.x >= b.end.x
            || this->end.y <= b.start.y
            || this->start.y >= b.end.y
            || this->end.z <= b.start.z
            || this->start.z >= b.end.z)
            return false;
        return true;
    }*/

    // AABB OVERLAP
    /*bool Overlap(const AABB& b) const {
        if (this->end.x < b.start.x) return false;
        if (this->start.x > b.end.x) return false;
        if (this->end.y < b.start.y) return false;
        if (this->start.y > b.end.y) return false;
        if (this->end.z < b.start.z) return false;
        if (this->start.z > b.end.z) return false;
        return true;
    }*/
    // becuase of floating point errors
    bool Overlap(const AABB& b) const {
        constexpr float EPS = 1e-5f;
        if (this->end.x <= b.start.x + EPS) return false;
        if (this->start.x >= b.end.x - EPS) return false;
        if (this->end.y <= b.start.y + EPS) return false;
        if (this->start.y >= b.end.y - EPS) return false;
        if (this->end.z <= b.start.z + EPS) return false;
        if (this->start.z >= b.end.z - EPS) return false;
        return true;
    }

};




