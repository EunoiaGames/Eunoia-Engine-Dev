#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

layout(set = 0, binding = 0) uniform PerFrame
{
	mat4 ViewProjection;
};

void main()
{
	vec2 newPos = (vec4(Pos, 1.0, 1.0) * ViewProjection).xy;
    gl_Position = vec4(newPos, 0.0, 1.0);
}

#EU_Fragment
#version 450

layout(location = 0) out float OutColor;

void main()
{
	OutColor = 1.0;
}