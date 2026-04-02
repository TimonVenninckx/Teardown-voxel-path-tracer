#include "ShadowMapCube.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ShadowMapCube::ShadowMapCube(unsigned int width, unsigned int height)
    : cubemap(width, height)
{
}


void ShadowMapCube::DrawMap(const glm::vec3& lightPosition, Shader& shader, 
    Framebuffer& frameBuffer)
{
    shader.Activate();
    frameBuffer.Bind();
    glViewport(0, 0, cubemap.width, cubemap.height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemap.ID, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    float cubeDepthAspect = (float)cubemap.width / (float)cubemap.height;

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.f), cubeDepthAspect,
        nearPlane, farPlane);
    shadowTransforms[0] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms[1] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms[2] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms[3] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms[4] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms[5] = glm::mat4(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


    for (unsigned int i = 0; i < 6; ++i)
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
    glUniform3fv(glGetUniformLocation(shader.ID, "lightPos"), 1, glm::value_ptr(lightPosition));
    glUniform1f(glGetUniformLocation(shader.ID, "far_plane"), farPlane);
}

void ShadowMapCube::Bind(int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.ID);
}

void ShadowMapCube::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
