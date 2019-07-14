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
layout(location = 2) in vec2 fragMipTexCoord;

layout(binding = 2) uniform sampler2D diffuseTex;
layout(binding = 3) uniform sampler2D diffuseMipTex; // texture built up of colour to represent the level.

// frag
layout(location = 0) out vec4 outColour;

// few extra resources:
// http://www.cs.cmu.edu/afs/cs/academic/class/15462-s16/www/lec_slides/07_texture.pdf
// https://stackoverflow.com/questions/24388346/how-to-access-automatic-mipmap-level-in-glsl-fragment-shader-texture
/*
	float mip_map_level(in vec2 texture_coordinate) // in texel units
	{
		vec2  dx_vtc        = dFdx(texture_coordinate);
		vec2  dy_vtc        = dFdy(texture_coordinate);
		float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
		float mml = 0.5 * log2(delta_max_sqr);
		return max( 0, mml );
	}

	// convert normalized texture coordinates to texel units before calling mip_map_level
    float mipmapLevel = mip_map_level(textureCoord * textureSize(myTexture, 0));
*/

void main() 
{
    vec4 col = texture(diffuseTex, fragTexCoord);
	vec4 mip = texture(diffuseTex, fragMipTexCoord);

	// Red = Too much detail
	// Tex Col = About right
	// Blue = Not enough
	outColour = vec4(mix(col.rgb, mip.rgb, mip.a), 1.0f);
}