#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UViewportData
{
    vec2 Dimensions;
} ViewportData;

layout(binding = 3) uniform UDirectionalLight
{
    vec3 Direction;
} DirectionalLight;

layout(push_constant) uniform UTimeData 
{
    float Time;
} TimeData;

// vert
layout(location = 0) in vec3 fragColour;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec3 fragWorldNor;

layout(binding = 2) uniform sampler2D diffuseTex;

// frag
layout(location = 0) out vec4 outColour;

#define EPSILON 0.0001f
#define SATURATE(x) clamp(x, EPSILON, 1.0f)
#define AMBIENT_TERM(x) x * 0.3f
#define SUM_KDKS(kdm, kdl, ksl, ksm, kse) kdm * kdl + (ksm * kse) * ksl
#define SUM_KD(kdm, kdl) kdm * kdl

void main() 
{
	vec3 normal = fragWorldNor;

	vec3 camera_position = vec3(2.0f, 0.0f, 2.0f); // same position lookat() was generated from
	vec3 camera_direction = normalize(camera_position - fragWorldPos);
	
	vec3 sun_colour = vec3(1.0f);
	vec3 sun_direction = DirectionalLight.Direction;
	float sun_distance = 50.0f;
	float sun_strength = 50.0f;
	
	vec3 halfway = normalize(sun_direction + camera_direction);
	
	vec3 diffuse = (sun_colour * (SATURATE(dot(normal, sun_direction))) / sun_distance) * sun_strength;
	diffuse += AMBIENT_TERM(diffuse);

	vec3 tex_colour = texture(diffuseTex, fragTexCoord).rgb;
	outColour = vec4(SUM_KD(tex_colour, diffuse), 1.0f);
}