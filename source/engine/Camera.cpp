#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Engine.h"

Camera::Camera(int width, int height, glm::vec3 position, float FOVdeg, float nearPlane, float farPlane)
{
    this->width = width;
    this->height = height;
    this->position = position;
    
    UpdateProjection(FOVdeg, nearPlane, farPlane);
}


Frustum Camera::CreateFrustum()
{
    //Frustum frustum;
    
    //const float halfVSide = far * tanf(fovY * .5f);
    //const float halfHSide = halfVSide * (float)width / (float)height;
    //const glm::vec3 frontMultFar = far * front;
    //
    //frustum.nearFace = { position + near * front, front };
    //frustum.farFace = { position + frontMultFar, -front };
    //frustum.rightFace = { position,
    //                        glm::cross(frontMultFar - right * halfHSide, up) };
    //frustum.leftFace = { position,
    //                        glm::cross(up,frontMultFar + right * halfHSide) };
    //frustum.topFace = { position,
    //                        glm::cross(right, frontMultFar - up * halfVSide) };
    //frustum.bottomFace = { position,
    //                        glm::cross(frontMultFar + up * halfVSide, right) };
    //
    

    
    Frustum f;

    // by chatgpt (obviously)
    // the first attempt had a lot of issues
    // next block i will tackle this myself for sure
    // 
    // Build view matrix from camera members (make sure front/up are correct)

    // Combined view-projection matrix (projection then view)

    // We'll extract planes from the vp matrix.
    // Note: glm is column-major. Access rows via glm::row() helper or manually.
    // We'll treat glm::mat4 m; then row i is:
    // glm::vec4 row0 = glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]); // not convenient
    // Instead use direct element access but easier: build rows like below:

    glm::vec4 row0 = glm::vec4(cameraMatrix[0][0], cameraMatrix[1][0], cameraMatrix[2][0], cameraMatrix[3][0]);
    glm::vec4 row1 = glm::vec4(cameraMatrix[0][1], cameraMatrix[1][1], cameraMatrix[2][1], cameraMatrix[3][1]);
    glm::vec4 row2 = glm::vec4(cameraMatrix[0][2], cameraMatrix[1][2], cameraMatrix[2][2], cameraMatrix[3][2]);
    glm::vec4 row3 = glm::vec4(cameraMatrix[0][3], cameraMatrix[1][3], cameraMatrix[2][3], cameraMatrix[3][3]);

    // Extract planes in the form (a,b,c,d) where plane equation is a*x + b*y + c*z + d = 0
    auto makePlane = [&](const glm::vec4& p) -> Plane {
        glm::vec3 n = glm::vec3(p.x, p.y, p.z);
        float length = glm::length(n);
        if (length == 0.0f) length = 1.0f;
        n /= length;
        float d = p.w / length;
        // We want distance stored as dot(normal, pointOnPlane), but we have plane as (n, d)
        // For plane ax+by+cz + d = 0, rearrange: dot(n, p) = -d
        Plane plane;
        plane.normal = n;
        plane.distance = -d; // so getSignedDistanceToPlane(point) = dot(n, point) - distance
        return plane;
        };

    // Left   = row3 + row0
    f.leftFace = makePlane(row3 + row0);
    // Right  = row3 - row0
    f.rightFace = makePlane(row3 - row0);
    // Bottom = row3 + row1
    f.bottomFace = makePlane(row3 + row1);
    // Top    = row3 - row1
    f.topFace = makePlane(row3 - row1);
    // Near   = row3 + row2
    f.nearFace = makePlane(row3 + row2);
    // Far    = row3 - row2
    f.farFace = makePlane(row3 - row2);

    return f;
    
}

void Camera::UpdateProjection(float FOVdeg, float nearPlane, float farPlane)
{
    this->fovY = FOVdeg;
    this->near = nearPlane;
    this->far = farPlane;


    //Engine::Instance().window->GetWindowSize(width,height);

    projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);
}

const glm::mat4& Camera::GetMatrix()
{
    if (matrixNeedsRebuild) {
        UpdateProjection(fovY, near, far);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(position, position + front, up);
        
        matrixNeedsRebuild = false;
        cameraMatrix = projection * view;
    }
    return cameraMatrix;
}

const glm::mat4& Camera::GetProjectionMatrix()
{
    return projection;
}

void Camera::SetOrientation(glm::vec3 orient)
{
    front = orient;
    right = glm::normalize(glm::cross(front, up));
    matrixNeedsRebuild = true;
}

void Camera::SetPosition(glm::vec3 pos){
    position = pos;
    matrixNeedsRebuild = true;
}

const glm::vec3 Camera::GetPosition()
{
    return position;
}

const glm::vec3 Camera::GetOrientation()
{
    return front;
}


//RayTracePlane Camera::GetRayTracePlane()
//{
//    RayTracePlane p;
//    float aspect = float(width) / float(height);
//    float halfH = tan(glm::radians(fovY) * 0.5f);  // screen half-height at distance 1
//    float halfW = halfH * aspect;
//
//    glm::vec3 realUp = glm::normalize(glm::cross(front, right));
//
//    p.topLeft = position + front - halfW * right + halfH * realUp;
//    p.topRight = position + front + halfW * right + halfH * realUp;
//    p.bottomLeft = position + front - halfW * right - halfH * realUp;
//
//    return p;
//}

RayTracePlane Camera::GetRayTracePlane()
{
    RayTracePlane p;

    float aspect = float(width) / float(height);

    glm::vec3 realUp = glm::normalize(glm::cross(right, front));
    p.camPos    = position;
    p.topLeft   = position + 2.f * front - aspect * right + realUp;
    p.topRight  = position + 2.f * front + aspect * right + realUp;
    p.bottomLeft= position + 2.f * front - aspect * right - realUp;
    
    return p;
}
