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

layout(set = 0, binding = 0) uniform sampler2D Texture;

const float Weights[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

void main()
{
	vec2 texOffset = 1.0 / textureSize(Texture, 0);
	vec3 result = texture(Texture, TexCoord0).rgb * Weights[0];

	for(int i = 1; i < 5; i++)
	{
		result += texture(Texture, TexCoord0 + vec2(texOffset.x * i, 0.0)).rgb * Weights[i];
		result += texture(Texture, TexCoord0 - vec2(texOffset.x * i, 0.0)).rgb * Weights[i];
	}
	
	OutColor = vec4(result, 1.0);
}