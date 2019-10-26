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

	vec4 shadowCoord;
	
	vec2 textCoord[2];
	
	mat3 TBN;
} dataOut;

uniform struct SpotLight
{
	vec3 wcs_pos;
	vec3 wcs_dir;
	
	float outerCutOff;
	float innerCutOff;
	
	vec3 color;
	float intensity;

	int attenCoef;

	mat4 mvpBiased; // transforms to [0,1] in light space
	
	bool castsShadow;
	
	int samples;
	float maxShadowBias;
	sampler2DShadow shadowMap;
} spotLight;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normalMatrix; 

void main()
{
    gl_Position = mvp * vec4(ocs_pos, 1.0);
	
	dataOut.textCoord[0] = textCoord0; 
	dataOut.textCoord[1] = textCoord1; 
	
	vec3 T = normalize(normalMatrix * ocs_tangent);
    vec3 B = normalize(normalMatrix * ocs_bitangent);
    vec3 N = normalize(normalMatrix * ocs_normal);

    dataOut.TBN = mat3(T, B, N);
	
	dataOut.wcs_fragPos = vec3(m * vec4(ocs_pos, 1.0));
	
	dataOut.shadowCoord = spotLight.mvpBiased * vec4(ocs_pos, 1.0);
}