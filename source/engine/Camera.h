#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "AABB.h"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Plane
{
    glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
    float     distance = 0.f;        // Distance with origin

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {
    }
    
    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }

    bool isOnOrForwardPlane(const AABB& aabb) const
    {
        glm::vec3 extents = glm::vec3(aabb.end - aabb.start) * 0.5f;
        //glm::vec3 extents = glm::vec3(CHUNKSIZE, CHUNKHEIGHT, CHUNKSIZE);
        glm::vec3 center = aabb.start + extents;

        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = extents.x * std::abs(normal.x) +
            extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);

        return -r <= getSignedDistanceToPlane(center);
    }

};


struct Frustum {
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;
    
    Plane farFace;
    Plane nearFace;

    bool isOnFrustum(const AABB& aabb) {
        return (leftFace.isOnOrForwardPlane(aabb) &&
            rightFace.isOnOrForwardPlane(aabb) &&
            topFace.isOnOrForwardPlane(aabb) &&
            bottomFace.isOnOrForwardPlane(aabb) &&
            nearFace.isOnOrForwardPlane(aabb) &&
            farFace.isOnOrForwardPlane(aabb));
    }
};

//https://jacco.ompf2.com/2024/05/22/ray-tracing-with-voxels-in-c-series-part-5/
struct RayTraceFrustum {
    glm::vec4 left{};
    glm::vec4 right{};
    glm::vec4 top{};
    glm::vec4 bottom{};
};

struct RayTracePlane {
    glm::vec3 camPos{};
    glm::vec3 topLeft{};
    glm::vec3 topRight{};
    glm::vec3 bottomLeft{};


    inline float distance(glm::vec4& plane, glm::vec3& pos)
    {
        return glm::dot(glm::vec3(plane), pos) - plane.w;
    }

    RayTraceFrustum CreateFrustum() {
        RayTraceFrustum f{};
        
        f.left    = glm::vec4(glm::cross(topLeft   - bottomLeft, topLeft - camPos),0.f);
        f.right   = glm::vec4(glm::cross(topRight  - camPos, topLeft - bottomLeft),0.f);
        f.top     = glm::vec4(glm::cross(topRight  - topLeft, topLeft - camPos),0.f);
        f.bottom  = glm::vec4(glm::cross(bottomLeft- camPos, topRight - topLeft),0.f);

        f.left.w   = distance(f.left, camPos);
        f.right.w  = distance(f.right, camPos);
        f.top.w    = distance(f.top, camPos);
        f.bottom.w = distance(f.bottom, camPos);

        return f;
    }

    static RayTracePlane CreateRayTracePlane(glm::vec3 position, glm::vec3 front, int width, int height)
    {
        RayTracePlane p;

        const float aspect = float(width) / float(height);

        constexpr glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
        const glm::vec3 right = glm::normalize(glm::cross(front, up));
        const glm::vec3 realUp = glm::normalize(glm::cross(right, front));

        p.camPos = position;
        p.topLeft = position + 2.f * front - aspect * right + realUp;
        p.topRight = position + 2.f * front + aspect * right + realUp;
        p.bottomLeft = position + 2.f * front - aspect * right - realUp;

        return p;
    }
};



class Camera {
public:
    
    
    Camera(int width, int height, glm::vec3 position, float FOVdeg, float nearPlane, float farPlane);

    Frustum CreateFrustum();

    void UpdateProjection(float FOVdeg, float nearPlane, float farPlane);
    const glm::mat4& GetMatrix();
    const glm::mat4& GetProjectionMatrix();
    void SetOrientation(glm::vec3 front);
    void SetPosition(glm::vec3 pos);
    const glm::vec3 GetPosition();
    const glm::vec3 GetOrientation();

    RayTracePlane GetRayTracePlane();

private:
    glm::vec3 position{};
    glm::vec3 front = glm::vec3(0.f, 0.f,-1.f);
    glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 up    = glm::vec3(0.f, 1.f, 0.f);


    float near{0.1f};
    float far{100.f};
    float fovY{45.f};

    float yaw{};
    float pitch{};
    
    int width{};
    int height{};

    bool matrixNeedsRebuild{ true };
    glm::mat4 projection{ glm::mat4(1.0f) };
    glm::mat4 cameraMatrix{ glm::mat4(1.f) };

};