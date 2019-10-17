#version 460 core

in Data
{ 
	vec2 textCoord[2];
} dataIn;

uniform bool mask;
uniform float alphaCutoff;

uniform vec4 baseColorFactor;
uniform int baseColorTexcoordIndex;

layout(binding=0) uniform sampler2D baseColorSampler;

void main()
{             
	vec4 sampledBaseColor = texture(baseColorSampler, dataIn.textCoord[baseColorTexcoordIndex]);

	float opacity = sampledBaseColor.a * baseColorFactor.a;

	// mask mode and cutoff
	if(mask && opacity < alphaCutoff)
		discard;
}  