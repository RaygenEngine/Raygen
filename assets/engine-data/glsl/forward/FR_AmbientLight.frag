#version 460 core

out vec4 out_color;
  
in Data
{ 
	vec2 textCoord[2];
} dataIn;

uniform struct Material
{
	// factors
	vec4 baseColorFactor;
	vec3 emissiveFactor;
	float occlusionStrength;
	
	// text coord indices
	int baseColorTexcoordIndex;
	int emissiveTexcoordIndex;
	int occlusionTexcoordIndex;
	
	// samplers
	sampler2D baseColorSampler;
	sampler2D emissiveSampler;
	sampler2D occlusionSampler;
	
	// alpha mask
	float alphaCutoff;
	bool mask;

} material;

uniform vec3 ambient;

void main()
{
	// sample material textures
	vec4 sampledBaseColor = texture(material.baseColorSampler, dataIn.textCoord[material.baseColorTexcoordIndex]);
	
	float opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask && opacity < material.alphaCutoff)
		discard;
	
	vec3 albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	vec3 emissive = texture(material.emissiveSampler, dataIn.textCoord[material.emissiveTexcoordIndex]).rgb * material.emissiveFactor;
	float occlusion = texture(material.occlusionSampler, dataIn.textCoord[material.occlusionTexcoordIndex]).r;
	
	vec3 color = (albedo *  ambient) + emissive;
	color = mix(color, color * occlusion, material.occlusionStrength);
		
	out_color = vec4(color, opacity);
}


