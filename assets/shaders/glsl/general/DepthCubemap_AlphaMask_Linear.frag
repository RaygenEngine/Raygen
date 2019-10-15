#version 460 core

in Data
{ 
	vec2 textCoord[2];
	vec4 wcs_fragPos;
} dataIn;

uniform vec3 center;
uniform float far;

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
		
	 // get distance between fragment and light source
    float distance = length(dataIn.wcs_fragPos.xyz - center);
    
    // map to [0;1] range by dividing by far_plane
    distance = distance / far;
    
    // write this as modified depth
    gl_FragDepth = distance;
}  