#EU_Vertex
#version 450

layout(location = 0) in vec2 Pos;

void main()
{
	gl_Position = vec4(Pos, 0.0, 1.0);
}

#EU_Fragment
#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput Input;

layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform ThresholdBuffer
{
	float Threshold;
};

void main()
{
	float Gamma = 2.2;

	vec4 Color = subpassLoad(Input);
	float Brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
	
	//Brightness = Brightness / (Brightness + 1.0);
	//Brightness = pow(Brightness, 1.0 / Gamma);

	vec4 O;
	if(Brightness > Threshold)
	{
		O = Color;
	}
	else
	{
		O = vec4(0.0, 0.0, 0.0, 1.0);
	}
	OutColor = O;
}