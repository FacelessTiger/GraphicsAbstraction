#type vertex
#version 450

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}

#type fragment
#version 450

layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}