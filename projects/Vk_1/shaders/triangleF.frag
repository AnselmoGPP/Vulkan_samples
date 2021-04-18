#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in  vec3 fragColor;
layout(location = 0) out vec4 outColor;			// layout(location=0) specifies the index of the framebuffer (usually, there's only one).

void main()
{
	outColor = vec4(fragColor, 1.0);
}