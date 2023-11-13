#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

layout(location = 0) out vec2 TexCoord0;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
	TexCoord0 = Pos * 0.5 + vec2(0.5, 0.5);
}

#EU_Fragment
#version 450

layout(location = 0) in vec2 TexCoord0;

layout(location = 0) out vec4 OutColor;

layout(set = 0, binding = 0) uniform sampler2D HdrTexture;
layout(set = 0, binding = 1) uniform sampler2D BloomTexture;

void main()
{
	vec4 HdrSample = texture(HdrTexture, TexCoord0);
	vec4 BloomSample = texture(BloomTexture, TexCoord0);
	vec4 Color = vec4(HdrSample.rgb + BloomSample.rgb, HdrSample.a);

	float Gamma = 2.2;
	Color.rgb = Color.rgb / (Color.rgb + vec3(1.0, 1.0, 1.0));
	Color.rgb = pow(Color.rgb, vec3(1.0 / Gamma, 1.0 / Gamma, 1.0 / Gamma));

	OutColor = Color;
}