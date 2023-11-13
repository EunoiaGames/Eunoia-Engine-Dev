#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Color;

layout(location = 0) out vec2 TexCoord0;
layout(location = 1) out vec3 Color0;

layout(set = 0, binding = 0) uniform Test
{
	vec2 Offset;
	vec2 p0;
	vec3 UColor;
	float p1;
};

void main()
{
	gl_Position = vec4(Pos + Offset, 0.0, 1.0);
	TexCoord0 = TexCoord;
	Color0 = Color * UColor;
}

#EU_Fragment
#version 450

layout(location = 0) in vec2 TexCoord0;
layout(location = 1) in vec3 Color0;

layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform sampler2D Albedo;

void main()
{
	OutColor = vec4(Color0, 1.0) * texture(Albedo, TexCoord0);
}