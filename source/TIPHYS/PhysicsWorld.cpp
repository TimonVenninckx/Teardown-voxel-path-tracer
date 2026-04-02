#include "PhysicsWorld.h"

namespace TIPHYS {
    Box* PhysicsWorld::GetBody(uint id)
    {
        if (id >= physicsBodies.size())
            return nullptr;

        return &physicsBodies[id];
    }


    void PhysicsWorld::BroadPhase() {

        //#pragma omp parallel for schedule(dynamic)
        for (int a{ 0 }; a < physicsBodies.size(); a++) {
            Box* bodyA = &physicsBodies[a];
            const AABB& aabba = bodyA->GetAABB();
            for (int b{ a+1 }; b < physicsBodies.size(); b++) {
                Box* bodyB = &physicsBodies[b];
                const AABB& aabbb = bodyB->GetAABB();

                if (bodyA->IsStatic() && bodyB->IsStatic())
                    continue;

                if (!aabba.Overlap(aabbb))
                    continue;

                ArbiterKey key(bodyA, bodyB);
                Arbiter newArb(bodyA, bodyB, currentFrame);
                if (newArb.numContacts > 0) {
                    auto iter = arbiters.find(key);
                    if (iter == arbiters.end()) {
                        arbiters.insert(std::pair<ArbiterKey,Arbiter>(key,newArb));
                    }
                    else {
                        iter->second.Update(newArb.contacts, newArb.numContacts, currentFrame);
                    }
                }
                else {
                    arbiters.erase(key);
                }
            }
        }
    }

    

    void PhysicsWorld::Step(float dt) {
        
        currentFrame++;
        for (Box& box : physicsBodies) {
            box.hit = false;

            if (box.invMass == 0.f)
                continue;
            box.isDirty = true;
        }
        float invDelta = dt > 0.0f ? 1.0f / dt : 0.0f;
        BroadPhase();

        // integrate forces
        for (Box& box : physicsBodies) {
            if (box.invMass == 0.f)
                continue;
            box.invInertiaWorld = box.rotation * box.invInertiaLocal * glm::transpose(box.rotation);

            box.velocity += dt * (gravity + box.invMass * box.force);
            box.angularVelocity += dt * (box.invInertiaWorld * box.torque);
        }

        for (auto it = arbiters.begin(); it != arbiters.end();) {
            if (it->second.currentFrame != currentFrame)
                it = arbiters.erase(it);
            else {
                it->second.PreStep(invDelta);
                it++;
            }
        }

        for (int i{ 0 }; i < 10; i++) {
            for (auto& arb : arbiters) {
                arb.second.ApplyImpulse(dt);
            }
        }

        // integrate velocities
        for (Box& box : physicsBodies) {
            if (box.invMass == 0.f)
                continue;

            box.velocity        *= powf(0.98f, dt * 60.f);
            box.angularVelocity *= powf(0.95f, dt * 60.f);

            box.position += dt * box.velocity;
            box.rotation += Skew(box.angularVelocity) * box.rotation * dt;

            // https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
            box.rotation = Orthonormalize(box.rotation);

            box.force = glm::vec3(0.f);
            box.torque = glm::vec3(0.f);
        }
    }

    PhysicsBodyHandle PhysicsWorld::AddBox(Box b) {
        b.invMass = 1.f / b.mass;

        //b.inertia = s.mass * (b.size.x * size.x + size.y * size.y) / 12.f;

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

        b.friction = 0.3f;
        physicsBodies.push_back(b);

        b.GetAABB();
        return PhysicsBodyHandle(this, static_cast<uint>(physicsBodies.size()) - 1);
    }

    PhysicsBodyHandle PhysicsWorld::AddStaticBox(Box b)
    {

        b.mass = MAX_FLOAT;
        b.invMass = 0.f;

        b.invInertiaLocal = glm::mat3(0.f);
        b.invInertiaWorld = glm::mat3(0.f);
        b.friction = 0.8f;

        physicsBodies.push_back(b);
        b.GetAABB();

        return PhysicsBodyHandle(this, static_cast<uint>(physicsBodies.size()) - 1);
    }

};