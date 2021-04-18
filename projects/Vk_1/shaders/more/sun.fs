// SUN (light source)

#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D sunTexture;

void main()
{
    FragColor = vec4(texture(sunTexture, TexCoord));
}








