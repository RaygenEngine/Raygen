#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 text_coord0;
layout (location = 5) in vec2 text_coord1;

out Data
{ 
// WIP: currently world space
	vec3 tangent_pos;
	vec3 tangent_view_pos;
	vec3 tangent_light_pos;
	
	vec2 text_coord[2];
	
	// WIP: tangent space only calcs in fragment shader
	mat3 TBN;
	
	// WIP: check what to do with this one
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
	
	dataOut.text_coord[0] = text_coord0; 
	dataOut.text_coord[1] = text_coord1; 
	
	vec3 T = normalize(normal_matrix * tangent.xyz);
    vec3 B = normalize(normal_matrix * bitangent);
    vec3 N = normalize(normal_matrix * normal);

    dataOut.TBN = mat3(T, B, N);
	
	// WIP actual tangent pos <- doesnt work currently
	dataOut.tangent_pos = vec3(m * vec4(pos,1));//dataOut.TBN * vec3(m * vec4(pos, 0.0));
	dataOut.tangent_view_pos  = view_pos;//dataOut.TBN * view_pos;
	dataOut.tangent_light_pos = light_pos;//dataOut.TBN * light_pos;
	
	dataOut.light_fragpos = light_space_matrix * vec4(dataOut.tangent_pos, 1.0);
}