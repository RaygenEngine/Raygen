#version 430 core

layout (location = 0) in vec3 ocs_pos;
layout (location = 1) in vec3 ocs_normal;
layout (location = 2) in vec3 ocs_tangent;
layout (location = 3) in vec3 ocs_bitangent;
layout (location = 4) in vec2 textCoord0;
layout (location = 5) in vec2 textCoord1;

out Data
{ 
	vec3 tcs_fragPos;
	vec3 tcs_viewPos;
	
	vec3 tcs_lightPos;
	vec3 tcs_lightDir;
	
	vec4 shadowCoord;
	
	vec2 textCoord[2];
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
	
	int samples;
	float maxShadowBias;
	sampler2DShadow shadowMap;
} spotLight;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normalMatrix; 

uniform vec3 wcs_viewPos;

void main()
{
    gl_Position = mvp * vec4(ocs_pos,1);
	
	dataOut.textCoord[0] = textCoord0; 
	dataOut.textCoord[1] = textCoord1; 
	
	vec3 T = normalize(normalMatrix * ocs_tangent);
    vec3 B = normalize(normalMatrix * ocs_bitangent);
    vec3 N = normalize(normalMatrix * ocs_normal);

    mat3 TBN = transpose(mat3(T, B, N));
	
	dataOut.tcs_fragPos = TBN * vec3(m * vec4(ocs_pos, 0.0));
	dataOut.tcs_viewPos  = TBN * wcs_viewPos;
	dataOut.tcs_lightPos = TBN * spotLight.wcs_pos;
	dataOut.tcs_lightDir = TBN * spotLight.wcs_dir;
	
	dataOut.shadowCoord = spotLight.mvpBiased * vec4(ocs_pos, 1.0);
}