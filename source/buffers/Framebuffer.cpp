#include "Framebuffer.h"

#include <glad/glad.h>
#include <iostream>


// by default it is a no draw/read buffer
Framebuffer::Framebuffer()
{   
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //    // woop woop
    //    printf("framebuffer INCOMPLETE!!!\n");
    //}
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}


void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
