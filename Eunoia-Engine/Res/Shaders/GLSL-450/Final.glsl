#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

layout(location = 0) out vec2 TexCoord0;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
	TexCoord0 = Pos * 0.5 + vec2(0.5, 0.5);
	TexCoord0.y = TexCoord0.y;
}

#EU_Fragment
#version 450

layout(location = 0) in vec2 TexCoord0;

layout(location = 0) out vec4 OutColor;

layout(set = 0, binding = 0) uniform sampler2D Sampler;

void main()
{
	OutColor = texture(Sampler, TexCoord0);
}
