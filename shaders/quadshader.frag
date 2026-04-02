#version 330
precision highp float;

uniform sampler2D image;

in vec2 UV;

out vec4 FragColor;

void main()
{
    FragColor = texture(image,UV);
}
