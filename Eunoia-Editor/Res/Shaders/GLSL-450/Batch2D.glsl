#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;
layout(location = 3) in float TextureID;

layout(location = 0) out vec2 TexCoord0;
layout(location = 1) out vec4 Color0;
layout(location = 2) out flat float TextureID0;

layout(set = 0, binding = 0) uniform PerFrame
{
	mat4 ViewProjection;
};

void main()
{
	vec2 newPos = (vec4(Pos, 1.0, 1.0) * ViewProjection).xy;
    gl_Position = vec4(newPos, 0.0, 1.0);
	TexCoord0 = TexCoord;
	Color0 = Color;
	TextureID0 = TextureID;
}

#EU_Fragment
#version 450

layout(location = 0) in vec2 TexCoord0;
layout(location = 1) in vec4 Color0;
layout(location = 2) in flat float TextureID0;

layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform sampler2D Textures[EU_MAX_SAMPLER_ARRAY_SIZE];

void main()
{
	vec4 texColor = texture(Textures[int(TextureID0)], TexCoord0);

	float Gamma = 2.2;
	OutColor = texColor * Color0;
}