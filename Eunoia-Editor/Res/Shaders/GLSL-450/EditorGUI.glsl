#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in uint Color;

layout(location = 0) out vec2 TexCoord0;
layout(location = 1) out vec4 Color0;

layout(set = 0, binding = 0) uniform PerFrame
{
	vec2 ScreenSize;
};

void main()
{
	TexCoord0 = TexCoord;
	
	Color0.a = ((Color >> 24) & 0xFF) / 255.0;
	Color0.r = ((Color >> 0) & 0xFF) / 255.0;
	Color0.g = ((Color >> 8) & 0xFF) / 255.0;
	Color0.b = ((Color >> 16) & 0xFF) / 255.0;

	vec2 NewPos = (Pos / ScreenSize) * 2.0 - vec2(1.0);
	gl_Position = vec4(NewPos, 0.0, 1.0);
}

#EU_Fragment
#version 450

layout(location = 0) in vec2 TexCoord0;
layout(location = 1) in vec4 Color0;

layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform sampler2D Sampler;

void main()
{
	vec4 TexSample = texture(Sampler, TexCoord0);
	float Alpha = TexSample.a;
	OutColor = vec4(TexSample.rgb * Color0.rgb, Alpha * Color0.a);
}