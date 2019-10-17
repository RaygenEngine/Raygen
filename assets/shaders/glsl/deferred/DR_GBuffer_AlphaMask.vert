#version 430 core

layout (location = 0) in vec3 ocs_pos;
layout (location = 1) in vec3 ocs_normal;
layout (location = 2) in vec3 ocs_tangent;
layout (location = 3) in vec3 ocs_bitangent;
layout (location = 4) in vec2 textCoord0;
layout (location = 5) in vec2 textCoord1;

out Data
{ 
	vec3 wcs_fragPos; 

	vec2 textCoord[2];
	
	mat3 TBN;
} dataOut;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normalMatrix; 

void main()
{
    gl_Position = mvp * vec4(ocs_pos,1);
	dataOut.wcs_fragPos = vec3(m * vec4(ocs_pos,1));
	dataOut.textCoord[0] = textCoord0; 
	dataOut.textCoord[1] = textCoord1; 
	
    vec3 T = normalize(normalMatrix * ocs_tangent);
    vec3 B = normalize(normalMatrix * ocs_bitangent);
    vec3 N = normalize(normalMatrix * ocs_normal);
	
    dataOut.TBN = mat3(T, B, N);
}