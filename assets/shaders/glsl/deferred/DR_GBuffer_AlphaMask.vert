#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 text_coord0;
layout (location = 5) in vec2 text_coord1;

out Data
{ 
	vec3 world_pos; 

	vec2 text_coord[2];
	
	mat3 TBN;
} dataOut;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normal_matrix; 

void main()
{
    gl_Position = mvp * vec4(pos,1);
	dataOut.world_pos = vec3(m * vec4(pos,1));
	dataOut.text_coord[0] = text_coord0; 
	dataOut.text_coord[1] = text_coord1; 
	
    vec3 T = normalize(normal_matrix * tangent.xyz);
    vec3 B = normalize(normal_matrix * bitangent);
    vec3 N = normalize(normal_matrix * normal);
	
    dataOut.TBN = mat3(T, B, N);
}