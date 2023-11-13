#EU_Vertex
#version 450

layout(location = 0) in vec3 Pos;

layout(set = 0, binding = 0) readonly buffer MVP
{
	mat4 Mvp;
};

void main()
{
	gl_Position = vec4(Pos, 1.0) * Mvp;
}

#EU_Fragment
#version 450

void main()
{
	
}