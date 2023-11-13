#EU_Vertex
#version 450

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec2 TexCoord;
layout(location = 5) in uvec4 BoneIDs;
layout(location = 6) in vec4 BoneWeights;

layout(location = 0) out vec3 Color0;
layout(location = 1) out vec3 Pos0;
layout(location = 2) out vec2 TexCoord0;
layout(location = 3) out vec3 Ambient0;
layout(location = 4) out vec3 CamPos0;
layout(location = 5) flat out uint EntityID0;
layout(location = 6) out mat3 TBN0;

layout(set = 0, binding = 0) uniform PerFrame
{
	mat4 ViewProjection;
	vec3 Ambient;
	float p0;
	vec3 CamPos;
	float p1;
};

layout(set = 1, binding = 0) uniform PerInstance
{
	mat4 Model;
	uint Animated;
	uint EntityID;
};

layout(std140, set = 1, binding = 1) readonly buffer BoneBuffer
{
	mat4 Bones[EU_RENDERER3D_MAX_BONES];
};

void main()
{
	mat4 BoneTransform = mat4(1 - Animated);
	for(uint i = 0; i < Animated * 4; i++)
	{
		BoneTransform += Bones[BoneIDs[i]] * (BoneWeights[i]);
	}


	vec3 AnimatedPos = (vec4(Pos, 1.0) * BoneTransform).xyz;
	vec3 AnimatedNormal = (vec4(Normal, 0.0) * BoneTransform).xyz;
	vec3 AnimatedTangent = (vec4(Tangent, 0.0) * BoneTransform).xyz;

	vec3 T = normalize(vec4(AnimatedTangent, 0.0) * Model).xyz;
	vec3 N = normalize(vec4(AnimatedNormal, 0.0) * Model).xyz;
	T = normalize(T - N * dot(T, N));
	vec3 B = cross(N, T);

	TBN0 = mat3(T, B, N);
	Pos0 = (vec4(AnimatedPos, 1.0) * Model).xyz;
	gl_Position = vec4(Pos0, 1.0) * ViewProjection;
	TexCoord0 = TexCoord;
	Color0 = Color;
	Ambient0 = Ambient;
	CamPos0 = CamPos;
	EntityID0 = EntityID;
}

#EU_Fragment
#version 450

layout(location = 0) in vec3 Color0;
layout(location = 1) in vec3 Pos0;
layout(location = 2) in vec2 TexCoord0;
layout(location = 3) in vec3 Ambient0;
layout(location = 4) in vec3 CamPos0;
layout(location = 5) flat in uint EntityID0;
layout(location = 6) in mat3 TBN0;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 OutAlbedo;
layout(location = 2) out vec4 OutPosition;
layout(location = 3) out vec4 OutNormal;

layout(set = 2, binding = 0) uniform sampler2D AlbedoMap;
layout(set = 2, binding = 1) uniform sampler2D NormalMap;
layout(set = 2, binding = 2) uniform sampler2D DisplacementMap;
layout(set = 2, binding = 3) uniform sampler2D MetallicMap;
layout(set = 2, binding = 4) uniform sampler2D RoughnessMap;

layout(set = 3, binding = 0) uniform MaterialModifier
{
	vec4 Albedo;
	float TexCoordScale;
	float AO;
	float DispScale;
	float DispOffset;
	float Metallic;
	float Roughness;
};

#include "../Mapping.glh"

void main()
{
	vec3 tangentViewDir = normalize((Pos0 - CamPos0) * TBN0);
	vec2 NewTexCoord = TexCoord0 * TexCoordScale;
	NewTexCoord = CalcParallaxTexCoords(NewTexCoord, tangentViewDir, DisplacementMap, DispScale, DispOffset);

	vec3 FinalAlbedo = texture(AlbedoMap, NewTexCoord).rgb * Albedo.rgb;
	vec3 Normal = texture(NormalMap, NewTexCoord).rgb;
	float FinalMetallic = texture(MetallicMap, NewTexCoord).r + Metallic;
	float FinalRoughness = texture(RoughnessMap, NewTexCoord).r + Roughness;

	Normal = Normal * 2.0 - vec3(1.0);
	Normal = TBN0 * normalize(Normal);

	float Gamma = 2.2;

	OutAlbedo = vec4(FinalAlbedo * Color0, 1.0f);
	OutAlbedo.rgb *= Albedo.rgb;
	OutAlbedo.rgb = pow(OutAlbedo.rgb, vec3(Gamma));
	OutAlbedo.a = EntityID0;
	FragColor = vec4(OutAlbedo.rgb * Ambient0 * AO, 1.0);
	OutPosition = vec4(Pos0, FinalMetallic);
	OutNormal = vec4(normalize(Normal), FinalRoughness);
}
