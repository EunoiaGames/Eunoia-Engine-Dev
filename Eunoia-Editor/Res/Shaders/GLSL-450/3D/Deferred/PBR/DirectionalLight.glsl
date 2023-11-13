#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
}

#EU_Fragment
#version 450

#include "LightCalcPBR.glh"

layout(set = 2, binding = 0) uniform LightBuffer
{
	DirectionalLight Light;
	mat4 LightMatrix;
};

#include "../Mapping.glh"

vec4 CalcDeferredPass(vec3 worldPos, vec3 camPos, vec3 normal, vec3 albedo, float metallic, float roughness)
{
	vec4 LightColor = CalcDirectionalLightPBR(Light, normal, camPos, worldPos, albedo, metallic, roughness);
	vec4 WorldPosLightSpace = vec4(worldPos, 1.0) * LightMatrix;

	return vec4(LightColor.rgb, 1.0);
}

#include "LightMain.glh"