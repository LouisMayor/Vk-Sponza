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

// frag
layout(location = 0) out vec4 outColour;

void main() 
{
	vec2 uv = gl_FragCoord.xy / ViewportData.Dimensions.xy;
	vec3 col = 0.5 + 0.5 * cos(TimeData.Time + uv.xyx + vec3(0,2,4));	

    outColour = vec4(col, 1.0);
}