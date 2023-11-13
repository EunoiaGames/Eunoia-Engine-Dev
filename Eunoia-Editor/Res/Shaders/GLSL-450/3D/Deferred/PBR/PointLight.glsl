#EU_Vertex
#version 450

layout(location = 0) in vec3 Pos;

layout(set = 2, binding = 0) uniform MVP
{
	mat4 Mvp;
};

void main()
{
	gl_Position = vec4(Pos, 1.0) * Mvp;
}

#EU_Fragment
#version 450

#include "LightCalcPBR.glh"

layout(set = 3, binding = 0) uniform Light
{
	PointLight light;
};

vec4 CalcDeferredPass(vec3 worldPos, vec3 camPos, vec3 normal, vec3 albedo, float metallic, float roughness)
{
	return CalcPointLightPBR(light, normal, camPos, worldPos, albedo, metallic, roughness);
}

#include "LightMain.glh"
