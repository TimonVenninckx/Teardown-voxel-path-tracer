#include "PhysicsWorld.h"
#include "Arbiter.h"

namespace TIPHYS {

    bool PositionCorrection = true;
    bool AccumulateImpulse = true;


    Arbiter::Arbiter(Box* b1, Box* b2, uint currentFrame)
    {
        if (b1 < b2) {
            body1 = b1;
            body2 = b2;
        }
        else {
            body1 = b2;
            body2 = b1;
        }
        this->currentFrame = currentFrame;

        numContacts = Collide(contacts, body1, body2);

        friction = sqrtf(body1->friction * body2->friction);
    }




    void Arbiter::PreStep(float /*invDelta*/)
    {
        Box* b1 = body1;
        Box* b2 = body2;

        for (int i{ 0 }; i < numContacts; i++) {
            Contact* c = contacts + i;

            c->r1 = c->position - b1->position;
            c->r2 = c->position - b2->position;

            const glm::vec3 tangent1 = ComputeTangent(c->normal);
            const glm::vec3 tangent2 = glm::cross(c->normal, tangent1);

            const glm::vec3 normalImpulse   = c->normal * c->accumulatedNormalLambda;
            const glm::vec3 frictionImpulse = tangent1 * c->accumulatedFrictionLambdas[0]
                                            + tangent2 * c->accumulatedFrictionLambdas[1];

            const glm::vec3 impulse = normalImpulse + frictionImpulse;
            b1->velocity -= impulse * b1->invMass;
            b1->angularVelocity -= b1->invInertiaWorld * glm::cross(c->r1, impulse);
            b2->velocity += impulse * b2->invMass;          
            b2->angularVelocity += b2->invInertiaWorld * glm::cross(c->r2, impulse);
        }
    }

    void Arbiter::Update(Contact* newContacts, int numNewContacts, uint curFrame)
    {
        this->currentFrame = curFrame;

        // can do stuff with featureID's for warm-starting
        // https://majikayogames.github.io/physics-tutorial/#equality-constraints
        Contact mergedContacts[Arbiter::MAX_CONTACT_POINTS]{};

        for (int i{ 0 }; i < numNewContacts; i++) {
            Contact* cNew = newContacts + i;

            int k = -1;
            for (int j{ 0 }; j < numContacts; ++j) {
                Contact* cOld = contacts + j;
                if (cNew->feature.value == cOld->feature.value) {
                    k = j;
                    break;
                }
            }
            if (k > -1) {
                Contact* c = mergedContacts + i;
                Contact* cOld = contacts + k;
                *c = *cNew;
                c->accumulatedFrictionLambdas[0] = cOld->accumulatedFrictionLambdas[0];
                c->accumulatedFrictionLambdas[1] = cOld->accumulatedFrictionLambdas[1];
                c->accumulatedNormalLambda   = cOld->accumulatedNormalLambda;
            }
            else {
                mergedContacts[i] = newContacts[i];
            }
        }
        for (int i = 0; i < numNewContacts; ++i)
            contacts[i] = mergedContacts[i];

        numContacts = numNewContacts;
    }

    

    void Arbiter::ApplyImpulse(float dt)
    {
        Box* b1 = body1;
        Box* b2 = body2;

        for (int i{ 0 }; i < numContacts; i++) {
            Contact* c = contacts + i;

            const glm::vec3 velA = b1->velocity + glm::cross(b1->angularVelocity, c->r1);
            const glm::vec3 velB = b2->velocity + glm::cross(b2->angularVelocity, c->r2);

            const glm::vec3 relVel = velB - velA;
            const float cDot = glm::dot(c->normal, relVel);

            glm::vec3 rnA = glm::cross(c->r1,c->normal);
            glm::vec3 rnB = glm::cross(c->r2,c->normal);
            const float effectiveMass = b1->invMass + b2->invMass
                + glm::dot(rnA, b1->invInertiaWorld * rnA)
                + glm::dot(rnB, b2->invInertiaWorld * rnB);
            if (effectiveMass < 0.000001f) 
                continue; // prevent division by 0


            constexpr float baumgarteFactor{0.1f};
                                                        // allowed penetration
            const float separation = std::min(0.f, -c->penetration + SLOP_LINEAR);
            const float velocityBias = (baumgarteFactor / dt) * separation;
            
            float lambda = -(cDot + velocityBias) / effectiveMass;

            // clamp accumulated impulse
            const float oldAccum = c->accumulatedNormalLambda;
            c->accumulatedNormalLambda = std::max(oldAccum + lambda, 0.f);
            lambda = c->accumulatedNormalLambda - oldAccum;

            if (lambda == 0) 
                continue;


            const glm::vec3 impulse = c->normal * lambda;
            b1->ApplyImpulse(-impulse, c->position);
            b2->ApplyImpulse( impulse, c->position);
        }
        SolveFriction();
    }

    void Arbiter::SolveFriction()
    {
        if (friction <= 0.f) return;

        
        Box* b1 = body1;
        Box* b2 = body2;

        for (int con{ 0 }; con < numContacts; con++) {
            Contact* c = contacts + con;
            const glm::vec3 velA = b1->velocity + glm::cross(b1->angularVelocity, c->r1);
            const glm::vec3 velB = b2->velocity + glm::cross(b2->angularVelocity, c->r2);

            const glm::vec3 relVel = velB - velA;
            glm::vec3 tangents[2];
            tangents[0] = ComputeTangent(c->normal);
            tangents[1] = glm::cross(c->normal, tangents[0]);
            
            // calculate accumulated friction lambda
            for (int t{ 0 }; t < 2; t++) {

                const glm::vec3 rnA = glm::cross(c->r1, tangents[t]);
                const glm::vec3 rnB = glm::cross(c->r2, tangents[t]);
                const float effectiveMassTangent = b1->invMass + b2->invMass
                    + glm::dot(rnA, b1->invInertiaWorld * rnA)
                    + glm::dot(rnB, b2->invInertiaWorld * rnB);
                if (effectiveMassTangent < 0.000001) continue;

                const float cDot = glm::dot(tangents[t], relVel);
                float lambda = -cDot / effectiveMassTangent;

                const float maxFriction = friction * c->accumulatedNormalLambda;

                // clamp force between -maxfriction and maxfricton;
                const float oldAccum = c->accumulatedFrictionLambdas[t];
                c->accumulatedFrictionLambdas[t] = std::max(-maxFriction, std::min(oldAccum + lambda, maxFriction));
                lambda = c->accumulatedFrictionLambdas[t] - oldAccum;

                const glm::vec3 frictionImpulse = tangents[t] * lambda;
                body1->ApplyImpulse(-frictionImpulse, c->position);
                body2->ApplyImpulse( frictionImpulse, c->position);
            }
        }
    }
}

