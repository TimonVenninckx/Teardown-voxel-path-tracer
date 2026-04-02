#pragma once

#include "Body.h"

namespace TIPHYS {

    struct Contact {

        glm::vec3 position{};
        // a -> b normal
        glm::vec3 normal{};
        float penetration{};

        // from body1 to contact point
        glm::vec3 r1{};
        // from body2 to contact point
        glm::vec3 r2{};

        float accumulatedNormalLambda{};
        float accumulatedFrictionLambdas[2]{};

        union ContactType {
            uint value{};
#pragma warning(push)
#pragma warning(disable : 4201)
            struct{
                uint8_t incIndex;
                uint8_t refIndex;
                bool referenceIsA;
            };
        }feature;
#pragma warning(pop)
    };
    constexpr float SLOP_LINEAR{ 0.04f };



    // sorted pointers for consistency of AvsB collision
    // this improves stability (box-2d)
    struct ArbiterKey {
        ArbiterKey(Box* b1, Box* b2) {
            if (b1 < b2)
            {
                body1 = b1; body2 = b2;
            }
            else
            {
                body1 = b2; body2 = b1;
            }
        }

        Box* body1;
        Box* body2;
    };

    struct Arbiter {

        Arbiter(Box* b1, Box* b2, uint currentFrame);

        void PreStep(float invDelta);
        void Update(Contact* newContacts, int numNewContacts, uint currentFrame);

        void ApplyImpulse(float deltaTime);

        void SolveFriction();

        static constexpr int MAX_CONTACT_POINTS{ 8 };
        Contact contacts[MAX_CONTACT_POINTS];
        int numContacts{};
        uint currentFrame{};

        Box* body1{};
        Box* body2{};

        // combined friction
        float friction{};
    };


    // This is used by std::set
    inline bool operator < (const ArbiterKey& a1, const ArbiterKey& a2)
    {
        if (a1.body1 < a2.body1)
            return true;

        if (a1.body1 == a2.body1 && a1.body2 < a2.body2)
            return true;

        return false;
    }

    int Collide(Contact* contacts, Box* body1, Box* body2);
}