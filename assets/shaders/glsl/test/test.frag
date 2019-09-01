#version 460 core
out vec4 out_color;
  
in Data
{ 
	vec3 pos;  
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
	vec2 textCoord0;
	vec2 textCoord1;
} dataIn;

uniform int mode;

uniform vec3 viewPos;

uniform vec4 baseColorFactor;
uniform vec3 emissiveFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform float normalScale;
uniform float occlusionStrength;
uniform int alphaMode;
uniform float alphaCutoff;
uniform int doubleSided;

uniform sampler2D baseColorSampler;
uniform sampler2D metallicRoughnessSampler;
uniform sampler2D emissiveSampler;
//uniform sampler2D normalSampler;
//uniform sampler2D occlusionSampler;

#define M_1_PIf 0.318309886183790671538f
#define M_PIf 3.14159265358979323846f

// very basic unoptimized surface test shader
void main()
{

	// TODO render to FBO, sample depth texture, show hdr skymap at far
	/*vec3 dir = normalize(FragPos - viewPos);

	//float theta = atan(dir.x, dir.z);
	//float phi = M_PIf * 0.5f - acos(dir.y);
	//float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	//float v = 0.5f * (1.0f + sin(phi));
	//if(depth)
	//out_color = vec4(vec3(texture(skyHDRSampler, vec2(u,v))), 1.0);*/

	const vec4 baseColor = texture(baseColorSampler, dataIn.textCoord0);
	const vec4 metallicRoughness = texture(metallicRoughnessSampler, dataIn.textCoord0);
	const vec4 emissive = texture(emissiveSampler, dataIn.textCoord0);
	//const vec4 bump = texture(bumpSampler, UV);
	//const vec4 specular_parameters = texture(specularParametersSampler, UV);

	switch (mode)
	{
		case 0: // base color map
			out_color = baseColor;
			break;
			
		case 1: // base color factor
			out_color = baseColorFactor;
			break;
			
		case 2: // base color final
			out_color = baseColor * baseColorFactor;
			break;
			
		case 3: // metallic map
			out_color = vec4(metallicRoughness.bbb, 1.0);
			break;
			
		case 4: // metallic factor
			out_color = vec4(metallicFactor);
			break;
			
		case 5: // metallic final
			out_color = vec4(metallicRoughness.bbb, 1.0) * metallicFactor;
			break;
			
		case 6: // roughness map
			out_color = vec4(metallicRoughness.ggg, 1.0);
			break;
			
		case 7: // roughness factor
			out_color = vec4(roughnessFactor);
			break;
			
		case 8: // roughness final
			out_color = vec4(metallicRoughness.ggg, 1.0) * roughnessFactor;
			break;
			
		case 9: // normal
			out_color = vec4(dataIn.normal, 1.0);
			break;
			
		case 10: // normal scale
			out_color = vec4(normalScale);
			break;
			
		case 11: // normal map TODO
			break;
			
		case 12: // normal final TODO
			//scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
			break;
			
		case 13: // tangent
			out_color = dataIn.tangent;
			break;
			
		case 14: // tangent handedness
			out_color = vec4(dataIn.tangent.a);
			break;
			
		case 15: // bitangent
			out_color = vec4(dataIn.bitangent, 1.0);
			break;
			
		case 16: // occlusion map TODO

			break;
			
		case 17: // occlusion strength
			out_color = vec4(occlusionStrength);
			break;
			
		case 18: // occlusion final TODO
			// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
			break;
			
		case 19: // emissive map TODO
			out_color = vec4(vec3(emissive), 1.0);
			break;
			
		case 20: // emissive factor
			out_color = vec4(emissiveFactor, 1.0);
			break;
			
		case 21: // emissive final
			out_color = vec4(vec3(emissive)*emissiveFactor, 1.0);
			break;
			
		case 22: // opacity map
			out_color = vec4(baseColor.a);
			break;
			
		case 23: // opacity factor
			out_color = vec4(baseColorFactor.a);
			break;
			
		case 24: // opacity final

			break;
			
		case 25: // texture coordinate 0
			out_color = vec4(dataIn.textCoord0, 0.0, 1.0);
			break;
			
		case 26: // texture coordinate 1
			out_color = vec4(dataIn.textCoord1, 0.0, 1.0);
			break;
			
		case 27: // alpha mode TODO

			break;
			
		case 28: // alpha cutoff TODO

			break;
	
		case 29: // alpha mask TODO

			break;
			
		case 30: // double sidedness TODO

			break;
	}
}