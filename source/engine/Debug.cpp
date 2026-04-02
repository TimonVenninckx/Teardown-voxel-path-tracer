#include "Debug.h"
#include "glm/vec3.hpp"
#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include "Shader.h"

bool debugShouldRebuild = false;

struct DebugObject {
    std::vector<DebugVertex> vertices;
    float lifetime{3.f};
};


std::vector<DebugObject> allDebugObjects;

Debug::Debug()
{
    debugShader = new Shader("shaders/debugshader.vert", "shaders/debugshader.frag");
}

Debug::~Debug()
{
    delete debugShader;
}

void Debug::DrawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float lifetime) {
    DebugObject dbo;
    dbo.lifetime = lifetime;

    //first bottom lines
    dbo.vertices.emplace_back(DebugVertex{ start, color });
    dbo.vertices.emplace_back(DebugVertex{ end, color });

    allDebugObjects.push_back(dbo);
    debugShouldRebuild = true;
}

void Debug::DrawBox(glm::vec3 center, glm::vec3 size, glm::vec3 color, float lifetime)
{
    // halfsize
    glm::vec3 hs = size * 0.5f;
    DebugObject dbo;
    dbo.lifetime = lifetime;

    //first bottom lines
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z - hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z - hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z + hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z + hs.z}, color });

    // top lines
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z - hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z - hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z + hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z + hs.z}, color });

    // side lines
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z + hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y - hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x + hs.x, center.y + hs.y, center.z - hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z + hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z + hs.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y - hs.y, center.z - hs.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {center.x - hs.x, center.y + hs.y, center.z - hs.z}, color });


    allDebugObjects.push_back(dbo);
    debugShouldRebuild = true;
}

void Debug::DrawAABB(AABB aabb, glm::vec3 color, float lifetime)
{
    DebugObject dbo;
    dbo.lifetime = lifetime;

    //first bottom lines
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.end.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.start.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.end.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.end.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.end.z}, color });

    // top lines
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.end.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.start.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.start.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.end.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.end.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.end.z}, color });

    // side lines
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.end.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.end.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.start.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.end.x, aabb.end.y, aabb.start.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.end.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.end.z}, color });

    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.start.y, aabb.start.z}, color });
    dbo.vertices.emplace_back(DebugVertex{ {aabb.start.x, aabb.end.y, aabb.start.z}, color });


    allDebugObjects.push_back(dbo);
    debugShouldRebuild = true;
}

void Debug::UpdateLifeTime(float deltaTime)
{
    for (std::vector<DebugObject>::iterator it{ allDebugObjects.begin() }; it != allDebugObjects.end();) {
        if (it->lifetime < deltaTime) {
            it = allDebugObjects.erase(it);
            debugShouldRebuild = true;
        }
        else {
            it->lifetime -= deltaTime;
            it++;
        }
    }
}

void Debug::RebuildBufferIfNeccessary()
{
    if (!debugShouldRebuild) {
        return;
    }
    std::vector<DebugVertex> vertices;
    
    for (DebugObject& db : allDebugObjects) {
        vertices.insert(vertices.end(), db.vertices.begin(), db.vertices.end());
    }
    
    vao.Bind();
    vbo.BufferData(vertices);
    // position, color
    vao.LinkVBO(vbo, 0, 3, GL_FLOAT, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
    vao.LinkVBO(vbo, 1, 3, GL_FLOAT, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));

    vao.Unbind();
    vbo.Unbind();

    verticeCount = static_cast<GLsizei>(vertices.size());
}

void Debug::RenderDebugObjects(const glm::mat4& camera)
{
    RebuildBufferIfNeccessary();
    if (verticeCount == 0)
        return;
    debugShader->Activate();
    debugShader->SetMat4("camMatrix", camera);

    glm::mat4 model = glm::mat4(1.f);
    debugShader->SetMat4("model", model);
    vao.Bind();
    glDrawArrays(GL_LINES, 0, verticeCount);
}
