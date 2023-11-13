#EU_Vertex
#version 450

layout(location = 0) in vec3 Pos;

layout(set = 1, binding = 0) uniform MVP
{
	mat4 Mvp;
};

void main()
{
	gl_Position = vec4(Pos, 1.0) * Mvp;
}

#EU_Fragment
#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput AlbedoSampler;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput PositionSampler;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput NormalSampler;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(0,0,0,0);
}