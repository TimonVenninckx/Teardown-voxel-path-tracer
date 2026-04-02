#include "PhysicsWorld.h"

#include "../engine/Engine.h"

namespace TIPHYS {


    std::vector<glm::vec3> ClipPolygonToPlane(const std::vector<glm::vec3>& points, glm::vec3 planeNormal, glm::vec3 planePoint) {
        // clipped points
        std::vector<glm::vec3> cp{};

        for (int i{ 0 }; i < points.size(); i++) {
            const glm::vec3& curr = points[i];
            const glm::vec3& next = points[(i + 1) % points.size()];

            const float dCurr = glm::dot(curr - planePoint, planeNormal);
            const float dNext = glm::dot(next - planePoint, planeNormal);

            if (dCurr >= 0) cp.emplace_back(curr);

            if (dCurr * dNext < 0) { // if negative they are both on different sides
                const float t = dCurr / (dCurr - dNext);
                cp.emplace_back(curr + t * (next - curr));
            }
        }
        return cp;
    }
    
    enum CollisionType {
        FACE_A,
        FACE_B,
        EDGE_EDGE
    };

    struct SATInfo{
        glm::vec3 normal{};
        float penetration{};
        bool overlapping{ false };
        CollisionType type;
        uint8_t axisIndexA{ 255 };
        uint8_t axisIndexB{ 255 };
        //int referenceIndex{};
        //bool referenceIsA{};
    };


    // returns radius of box's projection.
    float ProjectOBB(const Box& box, glm::vec3 axis) {
        return fabsf(glm::dot(axis, box.rotation[0]) * box.halfExtents.x)
             + fabsf(glm::dot(axis, box.rotation[2]) * box.halfExtents.z)
             + fabsf(glm::dot(axis, box.rotation[1]) * box.halfExtents.y);
    }


    SATInfo Check(const Box& a, const Box& b) {
        SATInfo info;
        info.penetration = MAX_FLOAT;
        info.overlapping = false;

        const glm::mat3& rotA = a.rotation;
        const glm::mat3& rotB = b.rotation;
        
        for (int i{ 0 }; i < 3; i++) {
            {
                const glm::vec3& axis = rotA[i];
                const float centerDist = fabsf(glm::dot(a.position, axis) - glm::dot(b.position, axis));
                const float seperation = centerDist - (ProjectOBB(a, axis) + ProjectOBB(b, axis));
                if (seperation > 0)
                    return info;

                const float penetration = -seperation;
                if (penetration < info.penetration) {
                    info.normal = axis;
                    info.penetration = penetration;
                    info.type = FACE_A;
                    info.axisIndexA = static_cast<uint8_t>(i);
                }
            }
            {
                const glm::vec3& axis = rotB[i];
                const float centerDist = fabsf(glm::dot(a.position, axis) - glm::dot(b.position, axis));
                const float seperation = centerDist - (ProjectOBB(a, axis) + ProjectOBB(b, axis));
                if (seperation > 0)
                    return info;

                const float penetration = -seperation;
                if (penetration < info.penetration) {
                    info.normal = axis;
                    info.penetration = penetration;
                    info.type = FACE_B;
                    info.axisIndexB = static_cast<uint8_t>(i);
                }
            }
        }
        for (int j{ 0 }; j < 3; j++) {
            for (int i{ 0 }; i < 3; i++) {
                const glm::vec3& notNormalized = glm::cross(rotA[i], rotB[j]);

                float len = glm::length(notNormalized);
                if (len < 0.0001f) continue;  // parallel edges, skip

                const glm::vec3 axis = notNormalized / len;
                const float centerDist = fabsf(glm::dot(a.position, axis) - glm::dot(b.position, axis));
                const float seperation = centerDist - (ProjectOBB(a, axis) + ProjectOBB(b, axis));
                if (seperation > 0)
                    return info;

                const float penetration = -seperation;
                if (penetration < info.penetration) {
                    info.normal = axis;
                    info.penetration = penetration;
                    info.type = EDGE_EDGE;
                    info.axisIndexA = static_cast<uint8_t>(i);
                    info.axisIndexB = static_cast<uint8_t>(j);
                }
            }
        }
        info.normal = glm::normalize(info.normal);
        info.overlapping = true;

        return info;


    }

    struct Face {
        glm::vec3 centerPoint;
        glm::vec3 points[4]{};
    };

    Face GetRefFace(Box& box,int axisIndex, glm::vec3 axis) {
        
        glm::vec3 faceNormal = box.rotation[axisIndex];

        if (glm::dot(faceNormal, axis) < 0)
            faceNormal = -faceNormal;

        glm::vec3 c = box.position + faceNormal * box.halfExtents[axisIndex];
        glm::vec3 u = box.rotation[(axisIndex + 1) % 3] * box.halfExtents[(axisIndex + 1) % 3];
        glm::vec3 v = box.rotation[(axisIndex + 2) % 3] * box.halfExtents[(axisIndex + 2) % 3];
        
        Face f{};
        f.centerPoint = c;
        f.points[0] = c + u - v;
        f.points[1] = c - u - v;
        f.points[2] = c - u + v;
        f.points[3] = c + u + v;


        {   // generate correct winding order, goofy fix;
            glm::vec3 e0 = f.points[1] - f.points[0];
            glm::vec3 e1 = f.points[3] - f.points[0];
            glm::vec3 computedNormal = glm::normalize(glm::cross(e0, e1));

            // if winding is backwards, reverse it
            if (glm::dot(computedNormal, faceNormal) > 0) {
                std::swap(f.points[1], f.points[3]); // flip winding
            }
        }

        return f;
    }

    Face GetIncFace(const Box& box, const glm::vec3& normal) {
        int bestAxis = 0;
        float mostNegative = MAX_FLOAT;
        float sign = 1;
        for (int i = 0; i < 3; i++) {
            float d = glm::dot(box.rotation[i], normal);
            
            // negative value, checking for both sides;
            // we want the most negative value as we want the face
            // the closest to the starting point of the normal.
            // hope this makes sense :P
            if (d < mostNegative) {
                sign = 1.f;
                mostNegative = d;
                bestAxis = i;
            }
            if (-d < mostNegative) {
                sign = -1.f;
                mostNegative = -d;
                bestAxis = i;
            }
        }
        glm::vec3 faceNormal = box.rotation[bestAxis] * sign;

        glm::vec3 c = box.position + faceNormal * box.halfExtents[bestAxis];
        glm::vec3 u = box.rotation[(bestAxis + 1) % 3] * box.halfExtents[(bestAxis + 1) % 3];
        glm::vec3 v = box.rotation[(bestAxis + 2) % 3] * box.halfExtents[(bestAxis + 2) % 3];

        Face f{};
        f.centerPoint = c;
        f.points[0] =  c + u - v;
        f.points[1] =  c - u - v;
        f.points[2] =  c - u + v;
        f.points[3] =  c + u + v;
        return f;
    }

    void GetSupportEdge(const Box& box, int axisIndex, glm::vec3 normal, glm::vec3& startOut, glm::vec3& endOut) {

        glm::vec3 edgeDir = box.rotation[axisIndex];

        // generate all 4 possible positions
        const int idx1 = (axisIndex + 1) % 3;
        const int idx2 = (axisIndex + 2) % 3;

        glm::vec3 u = box.rotation[idx1] * box.halfExtents[idx1];
        glm::vec3 v = box.rotation[idx2] * box.halfExtents[idx2];


        glm::vec3 candidates[4] = {
            box.position + u - v,
            box.position - u - v,
            box.position - u + v,
            box.position + u + v,
        };

        float best = -MAX_FLOAT;
        int bestIndex = 0;
        for (int i{ 0 }; i < 4; i++) {
            float d = glm::dot(candidates[i], normal);
            if (d > best) {
                best = d;
                bestIndex = i;
            }
        }

        glm::vec3 half = edgeDir * box.halfExtents[axisIndex];
        startOut = candidates[bestIndex] - half;
        endOut   = candidates[bestIndex] + half;
    }



    int Collide(Contact* contacts, Box* bodyA, Box* bodyB)
    {

        SATInfo info = Check(*bodyA, *bodyB);

        if (!info.overlapping)
            return 0;


        glm::vec3 refFaceCenter{};
        int contactPoints{ 0 };
        // from A->B
        glm::vec3 contactNormal{info.normal};
            // normal point from A->B
        if (glm::dot(contactNormal, bodyB->position - bodyA->position) < 0)
            contactNormal = -contactNormal;

        const glm::vec3 solverNormal = contactNormal;

        std::vector<glm::vec3> clippedPoints;
        //constexpr float displayTime{ 0.1f };
        switch (info.type) {
        case FACE_A: {

            Face refFace = GetRefFace(*bodyA, info.axisIndexA, contactNormal);
            Face incFace = GetIncFace(*bodyB, contactNormal);

            for (int f{ 0 }; f < 4 ; f++)
                clippedPoints.emplace_back(incFace.points[f]);

            for (int i{ 0 }; i < 4; i++) {
                const glm::vec3 edgeStart = refFace.points[i];
                const glm::vec3 edgeEnd   = refFace.points[(i+1) % 4];
                glm::vec3 edgeDir = glm::normalize(edgeEnd - edgeStart);
                // inward normal edge x faceNormal
                // will be used to clip
                glm::vec3 inwardNormal = glm::normalize(glm::cross(edgeDir, contactNormal));

                clippedPoints = ClipPolygonToPlane(clippedPoints, inwardNormal, edgeStart);
                /*glm::vec3 edgeMid = (edgeStart + edgeEnd) * 0.5f;
                Engine::Instance().debug->DrawLine(
                    edgeMid,
                    edgeMid + inwardNormal * 5.f,
                    glm::vec3(1, 1, 0), displayTime);*/

                if (clippedPoints.empty()) return 0;
            }

            refFaceCenter = refFace.centerPoint;
            break;
        }
        case FACE_B: {

            contactNormal = -contactNormal;
            
            Face refFace = GetRefFace(*bodyB, info.axisIndexB, contactNormal);
            Face incFace = GetIncFace(*bodyA, contactNormal);

            for (int f{ 0 }; f < 4; f++)
                clippedPoints.emplace_back(incFace.points[f]);

            for (int i{ 0 }; i < 4; i++) {
                const glm::vec3 edgeStart = refFace.points[i];
                const glm::vec3 edgeEnd = refFace.points[(i + 1) % 4];

                glm::vec3 edgeDir = glm::normalize(edgeEnd - edgeStart);
                // inward normal edge x faceNormal
                // will be used to clip
                glm::vec3 inwardNormal = glm::normalize(glm::cross(edgeDir, contactNormal));

                clippedPoints = ClipPolygonToPlane(clippedPoints, inwardNormal, edgeStart);

                if (clippedPoints.empty()) return 0;
            }

            refFaceCenter = refFace.centerPoint;
            break;
        }
        case EDGE_EDGE:
            //printf("Edge-ing\n");

            glm::vec3 startA, endA, startB, endB;

            GetSupportEdge(*bodyA, info.axisIndexA,  contactNormal, startA, endA);
            GetSupportEdge(*bodyB, info.axisIndexB, -contactNormal, startB, endB);

            //Engine::Instance().debug->DrawLine(startA, endA, glm::vec3(1.f, 0.f, 0.f), 0.5f);
            //Engine::Instance().debug->DrawLine(startB, endB, glm::vec3(1.f, 0.f, 0.f), 0.5f);


            // closest point between the two edges
            glm::vec3 d1 = endA - startA;
            glm::vec3 d2 = endB - startB;
            glm::vec3 r = startA - startB;

            float a = glm::dot(d1, d1);
            float e = glm::dot(d2, d2);
            float f = glm::dot(d2, r);
            float c = glm::dot(d1, r);
            float b = glm::dot(d1, d2);
            float denom = a * e - b * b;

            float s = (fabsf(denom) > 1e-6f) ? glm::clamp((b * f - c * e) / denom, 0.f, 1.f) : 0.f;
            float t = (b * s + f) / e;
            t = glm::clamp(t, 0.f, 1.f);
            s = glm::clamp((b * t - c) / a, 0.f, 1.f); // recompute s after clamping t

            glm::vec3 onA = startA + d1 * s;
            glm::vec3 onB = startB + d2 * t;
            glm::vec3 contactPoint = (onA + onB) * 0.5f;

            contacts[contactPoints].position = contactPoint;
            contacts[contactPoints].normal = solverNormal;
            contacts[contactPoints].penetration = info.penetration;
            contactPoints++;
            return contactPoints;
        }

        assert(glm::length(solverNormal) > 0.99f); // should always be unit length
        assert(glm::dot(solverNormal, bodyB->position - bodyA->position) >= 0.f);

        
        for (int i{ 0 }; i < clippedPoints.size(); i++) {

            // keep points that are behind the reference face, plus speculative slop box2D
            // allows for very small penetration.
            const float separation = glm::dot(clippedPoints[i] - refFaceCenter, contactNormal);
            if (separation < 0.f && contactPoints < Arbiter::MAX_CONTACT_POINTS) {

                const float penetration = -separation;
                //Engine::Instance().debug->DrawLine(bodyA->position, bodyA->position + contactNormal * 50.f, glm::vec3(1.f, 1.f, 1.f), displayTime);
                //Engine::Instance().debug->DrawBox(p, glm::vec3(1.f), glm::vec3(1.f, 1.f, 1.f), displayTime);

                Contact* c = contacts + contactPoints;
                c->position = clippedPoints[i];
                c->penetration = penetration;

                if (info.type == FACE_B) {
                    c->feature.referenceIsA = false;
                    c->feature.refIndex = info.axisIndexB;
                    c->feature.incIndex = static_cast<uint8_t>(i);

                }
                else {
                    c->feature.referenceIsA = true;
                    c->feature.refIndex = info.axisIndexA;
                    c->feature.incIndex = static_cast<uint8_t>(i);
                }
                c->normal = solverNormal;

                contactPoints++;
            }
        }


        return contactPoints;
    }

}