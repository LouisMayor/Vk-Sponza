#version 450
#extension GL_GOOGLE_include_directive : enable

#include "inputs.glsl"

layout(binding = 0) uniform UTranformData
{
    mat4 MVP;
	mat4 World;
} TranformData;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec3 fragWorldNor;

void main() 
{
    gl_Position = TranformData.MVP * vec4(inPosition, 1.0);
	fragWorldPos = (TranformData.World * vec4(inPosition, 1.0)).xyz;
	fragWorldNor = (TranformData.World * vec4(inNorCoord, 0.0)).xyz;
    fragColour = vec3(inColour);
	fragTexCoord = inTexCoord;
}