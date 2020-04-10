#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
// rgb: albedo, a: opacity
layout (location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec3 fragPos; 
	vec2 uv;
	mat3 TBN;
};

// uniforms

layout(set = 0, binding = 0) uniform UBO_Material {
	// factors
    vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;
} material;

layout(set = 0, binding = 1) uniform sampler2D baseColorSampler;
layout(set = 0, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 0, binding = 3) uniform sampler2D occlusionSampler;
layout(set = 0, binding = 4) uniform sampler2D normalSampler;
layout(set = 0, binding = 5) uniform sampler2D emissiveSampler;

void main() {
	// sample material textures
	vec4 sampledBaseColor = texture(baseColorSampler, uv);
	
	float opacity = sampledBaseColor.a * material.baseColorFactor.a;
	// mask mode and cutoff
	if(material.mask == 1 && opacity < material.alphaCutoff)
		discard;
	
	vec4 sampledMetallicRoughness = texture(metallicRoughnessSampler, uv); 
	vec4 sampledEmissive = texture(emissiveSampler, uv);
	vec4 sampledNormal = texture(normalSampler, uv);
	vec4 sampledOcclusion = texture(occlusionSampler, uv);
	
	// final material values
	vec3 albedo = sampledBaseColor.rgb * material.baseColorFactor.rgb;
	float metallic = sampledMetallicRoughness.b * material.metallicFactor;
	float roughness = sampledMetallicRoughness.g * material.roughnessFactor;
	vec3 emissive = sampledEmissive.rgb * material.emissiveFactor.rgb;
	float occlusion = sampledOcclusion.r;
	vec3 normal = normalize((sampledNormal.rgb* 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
	// opacity set from above

    // position
    gPosition = vec4(fragPos, 1.f);
	
    // normal (with normal mapping)
    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
	
    // albedo opacity
    gAlbedoOpacity = vec4(albedo, opacity);
	
	// spec params
	gSpecular = vec4(metallic, roughness, occlusion, material.occlusionStrength);
	
	// emissive
	gEmissive = vec4(emissive, 1.f);
}                                                                                        
                                                                                         
                                                                                          
                                                                                           
                                                                                            
                                                                                             
                                                                                              
                                                                                               
                                                                                           
                                                                                            
                                                                                             
                                                                                              
                                                                                               
                                                                                                
                                                                                                 
                                                                                                  
