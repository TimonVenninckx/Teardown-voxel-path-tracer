#include "ShadowMap2D.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../engine/Shader.h"
#include "Framebuffer.h"
#include <glm/common.hpp>

ShadowMap2D::ShadowMap2D(unsigned int width, unsigned int height)
    : texture(width, height)
{
}


void ShadowMap2D::DrawMap(const glm::vec3& direction, Shader& shader,
    Framebuffer& frameBuffer)
{
    shader.Activate();
    frameBuffer.Bind();
    glViewport(0, 0, texture.width, texture.height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.ID, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    glm::mat4 lightProjection = glm::ortho(-boxSize, boxSize, -boxSize, boxSize, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(glm::normalize(direction) * -1.f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
}

void ShadowMap2D::BindTexture(int slot) {
    texture.Bind(slot);
}

void ShadowMap2D::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
