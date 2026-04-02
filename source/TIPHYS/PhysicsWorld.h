#pragma once

#include "Body.h"
#include "Arbiter.h"
#include <map>

namespace TIPHYS {

    inline glm::mat2x2 CreateRotationMatrix(float angle) {
        glm::mat2x2 m;
        float c = cosf(angle);
        float s = sinf(angle);
        m[0].x = c;
        m[0].y = s;
        m[1].x = -s;
        m[1].y = c;
        return m;
    }


    // minimum translation vector.
    struct Collision {
        // B - A
        glm::vec2 normal{};
        bool isOverlapping{ false };
        float penetrationDepth{ MAX_FLOAT };
    };

    // projection onto axis.
    struct Projection {
        float min{ MAX_FLOAT };
        float max{-MAX_FLOAT };

        bool Overlap(const Projection& b)const {
            if (this->max < b.min
                || this->min > b.max)
                return false;

            return true;
        }
        float GetOverlap(const Projection& b)const {
            return std::min(this->max, b.max) - std::max(this->min, b.min);
            //return std::min(this->max - b.min, b.max - this->min);
        }

        bool Contains(const Projection& b)const {
            if (this->min < b.min && this->max > b.max)
                return true;
            return false;
        }
    };


    class PhysicsWorld
    {
    public:

        ~PhysicsWorld() {
        }

        // temp :( might need an unordered map to index when removing bodies.
        Box* GetBody(uint id);
 
        void BroadPhase();

        void Step(float deltaTime);

        PhysicsBodyHandle AddBox(Box b);

        PhysicsBodyHandle AddStaticBox(Box b);

        std::map<ArbiterKey, Arbiter> arbiters;

        int iterations{ 1 };
        glm::vec3 gravity{ 0.f, -9.81f,0.f};

        std::vector<Box> physicsBodies;

        uint currentFrame{};
    };
};