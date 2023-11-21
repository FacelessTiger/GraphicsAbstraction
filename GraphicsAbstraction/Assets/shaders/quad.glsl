#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

layout (location = 0) out vec3 v_Normal;
layout (location = 1) out vec3 v_FragPos;
layout (location = 2) out vec3 v_ViewPos;

layout(push_constant) uniform constants
{
	mat4 ViewProjection;
	mat4 Model;
	vec3 ViewPos;
} PushConstants;

void main()
{
	gl_Position = PushConstants.ViewProjection * PushConstants.Model * vec4(a_Position, 1.0);

	v_Normal = mat3(transpose(inverse(PushConstants.Model))) * a_Normal;
	v_FragPos = vec3(PushConstants.Model * vec4(a_Position, 1.0));
	v_ViewPos = PushConstants.ViewPos;
}

#type fragment
#version 450

layout (location = 0) in vec3 v_Normal;
layout (location = 1) in vec3 v_FragPos;
layout (location = 2) in vec3 v_ViewPos;

layout (location = 0) out vec4 o_Color;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	vec3 objectColor = vec3(1.0, 0.5, 0.31);
	vec3 lightPos = vec3(1.2, 1.0, 2.0);

	float ambientStength = 0.05;
	vec3 ambient = ambientStength * lightColor;

	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(lightPos - v_FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float specularStength = 0.5;

	vec3 viewDir = normalize(v_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	o_Color = vec4(result, 1.0);
}