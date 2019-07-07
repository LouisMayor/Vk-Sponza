#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UViewportData
{
    vec2 Dimensions;
} ViewportData;

layout(push_constant) uniform UTimeData 
{
    float Time;
} TimeData;

// vert
layout(location = 0) in vec3 fragColour;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 2) uniform sampler2D diffuseTex;

// frag
layout(location = 0) out vec4 outColour;

void main() 
{
	outColour = texture(diffuseTex, fragTexCoord);
}