#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textCoord0;
layout (location = 5) in vec2 textCoord1;

out Data
{ 
	vec3 pos;  
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
	vec2 textCoord0;
	vec2 textCoord1;
} dataOut;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normalMatrix; 

void main()
{
    gl_Position = mvp * vec4(pos,1);
	dataOut.pos = vec3(m * vec4(pos,1));
	dataOut.normal = normalMatrix * normal;
	dataOut.tangent = tangent; 
	dataOut.bitangent = bitangent; 
	dataOut.textCoord0 = textCoord0; 
	dataOut.textCoord1 = textCoord1; 
}