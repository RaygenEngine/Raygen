#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in mat4 instanceMatrix;
  
out vec3 FragPos;  
out vec3 Normal;
out vec2 UV;

uniform mat4 vp; 

void main()
{
    gl_Position = vp * instanceMatrix * vec4(pos,1);
	Normal = transpose(inverse(mat3(instanceMatrix))) * normal;
	UV = uv; 
	FragPos = vec3(instanceMatrix * vec4(pos,1));
}