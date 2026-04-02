#version 330
precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 camMatrix;


out vec3 Color;

void main()
{
    gl_Position = camMatrix * vec4(vec3(model * vec4(aPos,1.0)), 1.0);

    Color = aColor;
}