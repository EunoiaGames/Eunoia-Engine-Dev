#EU_Vertex
#version 450

layout(location = 0) in vec3 Pos;

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
};

void main()
{
	gl_Position = vec4(Pos, 1.0) * Model * ViewProjection;
}

#EU_Fragment
#version 450

layout(location = 0) out vec4 OutColor;

layout(set = 2, binding = 0) uniform Wireframe
{
	vec3 Color;
	float p0;
};

void main()
{
	OutColor = vec4(Color, 1.0);
}