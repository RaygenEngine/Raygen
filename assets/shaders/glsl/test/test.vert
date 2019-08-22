#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
  
out vec3 FragPos;  
out vec3 Normal;
out vec2 UV;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normal_matrix; 

void main()
{
    gl_Position = mvp * vec4(pos,1);
	Normal = normal_matrix * normal;
	UV = uv; 
	FragPos = vec3(m * vec4(pos,1));
}