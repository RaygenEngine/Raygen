#version 460 core

// awful shader with extreme branching
// but good enough for previewing stuff fast
out vec4 out_color;

in Data
{ 
	vec3 wcs_fragPos;
	vec3 wcs_normal;
	vec3 wcs_tangent;
	vec3 wcs_bitangent;
	vec2 textCoord[2];
	
	mat3 TBN;
} dataIn;

uniform struct Material
{
	// factors
	vec4 baseColorFactor;
	vec3 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;
	
	// text coord indices
	int baseColorTexcoordIndex;
	int metallicRoughnessTexcoordIndex;
	int emissiveTexcoordIndex;
	int normalTexcoordIndex;
	int occlusionTexcoordIndex;
	
	// samplers
	sampler2D baseColorSampler;
	sampler2D metallicRoughnessSampler;
	sampler2D emissiveSampler;
	sampler2D normalSampler;
	sampler2D occlusionSampler;
	
	// alpha mask
	float alphaCutoff;
	vec3 alphaMode;
	vec3 doubleSided;
} material;

uniform int previewMode;
#define PM_MATERIAL 0
#define PM_FACTORS 1
#define PM_GEOMETRY 2
#define PM_TEXTCOORD 3

uniform int previewTarget;
// Material
#define M_ALBEDO 0
#define M_METALLIC 1
#define M_ROUGHNESS 2
#define M_OCCLUSION 3
#define M_EMISSIVE 4
#define M_NORMALMAP 5
#define M_OPACITY 6
#define M_ALPHACUTOFF 7
#define M_ALPHAMODE 8
#define M_DOUBLESIDED 9
// Factors
#define F_ALBEDO 0
#define F_METALLIC 1
#define F_ROUGHNESS 2
#define F_EMISSIVE 3
#define F_NORMAL 4
#define F_OCCLUSION 5
#define F_OPACITY 6
// Geometry
#define G_POSITIONS 0
#define G_NORMALS 1
#define G_TANGENTS 2
#define G_BITANGENTS 3
#define G_FINALNORMALS 4
#define G_TEXTCOORD0 5
#define G_TEXTCOORD1 6
// TextCoord
#define T_ALBEDO 0
#define T_METALLIC 1
#define T_ROUGHNESS 2
#define T_EMISSIVE 3
#define T_NORMAL 4
#define T_OCCLUSION 5

void PreviewMaterial()
{
	switch (previewTarget)
	{
		case M_ALBEDO:
		vec4 sampledBaseColor = texture(material.baseColorSampler, dataIn.textCoord[material.baseColorTexcoordIndex]);
		out_color = vec4(sampledBaseColor.rgb * material.baseColorFactor.rgb, 1.0);
		break;
		
		case M_METALLIC:
		vec4 sampledMetallic = texture(material.metallicRoughnessSampler, dataIn.textCoord[material.metallicRoughnessTexcoordIndex]); 
		out_color = vec4(vec3(sampledMetallic.b * material.metallicFactor), 1.0);
		break;
		
		case M_ROUGHNESS:
		vec4 sampledRoughness = texture(material.metallicRoughnessSampler, dataIn.textCoord[material.metallicRoughnessTexcoordIndex]); 
		out_color = vec4(vec3(sampledRoughness.g * material.roughnessFactor), 1.0);
		break;
		
		case M_OCCLUSION:
		vec4 sampledOcclusion = texture(material.occlusionSampler, dataIn.textCoord[material.occlusionTexcoordIndex]);
		out_color = vec4(vec3(sampledOcclusion.r), 1.0);
		break;
		
		case M_EMISSIVE:
		vec4 sampledEmissive = texture(material.emissiveSampler, dataIn.textCoord[material.emissiveTexcoordIndex]);
		out_color = vec4(vec3(sampledEmissive.g * material.emissiveFactor), 1.0);
		break;
		
		case M_NORMALMAP:
		out_color = texture(material.normalSampler, dataIn.textCoord[material.normalTexcoordIndex]);
		break;
		
		case M_OPACITY:
		vec4 sampledBaseColorOpacity = texture(material.baseColorSampler, dataIn.textCoord[material.baseColorTexcoordIndex]);
		out_color = vec4(vec3(sampledBaseColorOpacity.a * material.baseColorFactor.a), 1.0);
		break;
		
		case M_ALPHACUTOFF:
		out_color = vec4(vec3(material.alphaCutoff), 1.0);
		break;
		
		case M_ALPHAMODE:
		out_color = vec4(material.alphaMode, 1.0);
		break;
		
		case M_DOUBLESIDED:
		out_color = vec4(material.doubleSided, 1.0);
		break;
	}
}

void PreviewFactors()
{
	switch (previewTarget)
	{
		case F_ALBEDO:
		out_color = vec4(material.baseColorFactor.rgb, 1.0);
		break;
		
		case F_METALLIC:
		out_color = vec4(vec3(material.metallicFactor), 1.0);
		break;
		
		case F_ROUGHNESS:
		out_color = vec4(vec3(material.roughnessFactor), 1.0);
		break;
		
		case F_EMISSIVE:
		out_color = vec4(material.emissiveFactor, 1.0);
		break;
		
		case F_NORMAL:
		out_color = vec4(vec3(material.normalScale), 1.0);
		break;
		
		case F_OCCLUSION:
		out_color = vec4(vec3(material.occlusionStrength), 1.0);
		break;
		
		case F_OPACITY:
		out_color = vec4(vec3(material.baseColorFactor.a), 1.0);
		break;
	}
}

void PreviewGeometry()
{	
	switch (previewTarget)
	{
		case G_POSITIONS:
		out_color = vec4(dataIn.wcs_fragPos, 1.0);
		break;
		
		case G_NORMALS:
		out_color = vec4(dataIn.wcs_normal, 1.0);
		break;
		
		case G_TANGENTS:
		out_color = vec4(dataIn.wcs_tangent, 1.0);
		break;
		
		case G_BITANGENTS:
		out_color = vec4(dataIn.wcs_bitangent, 1.0);
		break;
		
		case G_FINALNORMALS:
		vec4 sampledNormal = texture(material.normalSampler, dataIn.textCoord[material.normalTexcoordIndex]);
		vec3 scaledNormal = normalize((sampledNormal.rgb * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0));
		out_color = vec4(normalize(dataIn.TBN * scaledNormal), 1.0);
		break;
		
		case G_TEXTCOORD0:
		out_color = vec4(dataIn.textCoord[0], 0.0, 1.0);
		break;
		
		case G_TEXTCOORD1:
		out_color = vec4(dataIn.textCoord[1], 0.0, 1.0);
		break;
	}
}

void PreviewTexCoordIndex()
{
	switch (previewTarget)
	{
		case T_ALBEDO:
		out_color = vec4(vec3(material.baseColorTexcoordIndex), 1.0);
		break;
		
		case T_METALLIC:
		out_color = vec4(vec3(material.metallicRoughnessTexcoordIndex), 1.0);
		break;
		
		case T_ROUGHNESS:
		out_color = vec4(vec3(material.metallicRoughnessTexcoordIndex), 1.0);
		break;
		
		case T_EMISSIVE:
		out_color = vec4(vec3(material.emissiveTexcoordIndex), 1.0);
		break;
		
		case T_NORMAL:
		out_color = vec4(vec3(material.normalTexcoordIndex), 1.0);
		break;
		
		case T_OCCLUSION:
		out_color = vec4(vec3(material.occlusionTexcoordIndex), 1.0);
		break;
	}
}

void main()
{
	// magenta for missing
	out_color = vec4(1.0, 0.0, 1.0, 1.0);
	
	switch (previewMode)
	{
		case PM_MATERIAL: PreviewMaterial(); break;
		case PM_FACTORS: PreviewFactors(); break;
		case PM_GEOMETRY: PreviewGeometry(); break;
		case PM_TEXTCOORD: PreviewTexCoordIndex(); break;
	}
}