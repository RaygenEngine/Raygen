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
	vec3 world_normal;
	vec4 world_tangent;
	vec3 world_bitangent;

	vec3 tangent_pos;
	vec3 tangent_view_pos;
	vec3 tangent_light_pos;
	
	vec2 text_coord0;
	vec2 text_coord1;
	
	// TODO: tangent space only calcs in fragment shader
	mat3 TBN;
	
	vec4 light_fragpos;
} dataOut;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normal_matrix; 
uniform mat4 light_space_matrix;

uniform vec3 view_pos;
uniform vec3 light_pos;

void main()
{
    gl_Position = mvp * vec4(pos,1);
	dataOut.world_pos = vec3(m * vec4(pos,1));
	dataOut.world_normal = normal_matrix * normal;
	dataOut.world_tangent = tangent; 
	dataOut.world_bitangent = bitangent; 
	dataOut.text_coord0 = text_coord0; 
	dataOut.text_coord1 = text_coord1; 
	
    vec3 T = normalize(vec3(m * vec4(tangent.xyz, 0.0)));
    vec3 B = normalize(vec3(m * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(m * vec4(normal, 0.0)));
	
	// TODO: check if use instead of bitangent uploading
	// re-orthogonalize T with respect to N
	// T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	// B = cross(N, T);
	
    dataOut.TBN = mat3(T, B, N);
	
	// something is wrong here
	dataOut.tangent_pos = dataOut.TBN * vec3(m * vec4(pos, 0.0));
	dataOut.tangent_view_pos  = dataOut.TBN * view_pos;
	dataOut.tangent_light_pos = dataOut.TBN * light_pos;
	
	dataOut.light_fragpos = light_space_matrix * vec4(dataOut.world_pos, 1.0);
}