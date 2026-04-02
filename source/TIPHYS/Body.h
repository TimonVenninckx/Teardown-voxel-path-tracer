#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <limits>

typedef unsigned int uint;

namespace TIPHYS {

    class PhysicsWorld;
    class Box;
        
    struct PhysicsBodyHandle {
    public:
        PhysicsBodyHandle() {
            world = nullptr;
        }

        PhysicsBodyHandle(PhysicsWorld* physWorld, uint physID) {
            world = physWorld;
            id = physID;
        }
        Box* operator->();
        Box* Get();
    private:
        uint id{ UINT32_MAX };

        bool IsValid() {
            return id != UINT32_MAX && world;
        }

        PhysicsWorld* world{};
    };

    struct AABB {
        glm::vec3 start{};
        glm::vec3 end{};

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

    constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

    inline glm::mat3 Skew(glm::vec3 w) {
        return glm::mat3(
            0, w.z, -w.y,   // column 0
            -w.z, 0, w.x,   // column 1
            w.y, -w.x, 0      // column 2
        );
    }

    inline glm::mat3 Orthonormalize(const glm::mat3& mat) {
        //https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
        //https://interactivetextbooks.tudelft.nl/linear-algebra/Chapter7/GramSchmidt.html
        glm::mat3 m;
        // we trust axis X
        m[0] = glm::normalize(mat[0]);
        // gram schmidt fix
        // remove vector 0 part of vec 2
        m[2] = glm::normalize(mat[2] - glm::dot(mat[2], mat[0]) * mat[0]);
        // cross so always orthogonal.
        m[1] = glm::cross(m[2], m[0]);

        return m;
    }

    inline glm::vec3 ComputeTangent(glm::vec3& normal) {
        glm::vec3 helper = (fabsf(normal.x) < 0.9f)
            ? glm::vec3(1, 0, 0)
            : glm::vec3(0, 1, 0);
        return glm::normalize(glm::cross(normal,helper));
    }


    class Box { // OBB
    public:

        glm::vec3 position{};
        // get normals from a mat3
        glm::mat3 rotation{1.f};
        glm::vec3 halfExtents{1.f};

        glm::vec3 velocity{};
        glm::vec3 angularVelocity{};

        glm::vec3 force{};
        glm::vec3 torque{};

        float friction{ .2f };
        float mass{10.f};
        float invMass{};
        bool hit{ false };
        bool isDirty{ true };

        // local space.
        glm::mat3 invInertiaLocal{ 1.f };
        glm::mat3 invInertiaWorld{ 1.f };


        bool IsStatic() const{
            return invMass == 0.f;
        }
        void AddForce(glm::vec3 f) {
            force += f;
        }
        void ApplyImpulse(glm::vec3 impulse, glm::vec3 worldPosition) {
            const glm::vec3 r = worldPosition - position;

            velocity += impulse * invMass;
            angularVelocity += invInertiaWorld * glm::cross(r, impulse);
        }

        glm::vec3 GetCorner(uint i, const glm::mat3& rot)const {
            return position + rot * glm::vec3(
                (i & 1) ? halfExtents.x : -halfExtents.x,
                (i & 2) ? halfExtents.y : -halfExtents.y,
                (i & 4) ? halfExtents.z : -halfExtents.z
            );
        }
        /*void UpdateTransform() {
            worldTransform = glm::translate(glm::scale(glm::mat4(rotation), halfExtents), position);
        }*/

        glm::mat4 GetWorldTransform()const { // scale-rotate-translate
            glm::mat4 trans = glm::mat4(rotation);
            trans[3] = glm::vec4(position, 1.0f);

            return glm::scale(trans,halfExtents);
        }

        const AABB& GetAABB() {
            if (isDirty) {
                glm::vec3 min = glm::vec3(MAX_FLOAT);
                glm::vec3 max = glm::vec3(-MAX_FLOAT);

                for (int i{ 0 }; i < 8; i++) {
                    glm::vec3 corner = GetCorner(i, rotation);
                    min = glm::min(min, corner);
                    max = glm::max(max, corner);
                }
                aabb.start = min;
                aabb.end = max;
                isDirty = false;
            }
            return aabb;
        }

    private:
        AABB aabb{};
    };

}