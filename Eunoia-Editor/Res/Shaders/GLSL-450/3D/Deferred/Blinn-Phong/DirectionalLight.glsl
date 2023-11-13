#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
}

#EU_Fragment
#version 450

#include "LightCalcBlinnPhong.glh"

layout(set = 2, binding = 0) uniform Light
{
	DirectionalLight light;
};

vec4 CalcDeferredPass(vec3 worldPos, vec3 camPos, vec3 normal, vec3 albedo, float specular, float gloss)
{
	return CalcDirectionalLightBlinn(light, normalize(normal), camPos, worldPos, albedo, specular, gloss);
}

#include "LightMain.glh"