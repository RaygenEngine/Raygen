// Raygen exported generated shader backup
//@ UBO Section:
float HeightMultiplier;
float flat_UvScale;
float slope_UvScale;
float worldSize;
float quads;
float anglePow;
float angleLow;
float angleHigh;

ubo ubo;

sampler2d heightmap;
sampler2d flat_Albedo;
sampler2d flat_Normal;
sampler2d flat_Metal;
sampler2d flat_Rough;
sampler2d slope_Albedo;
sampler2d slope_Normal;
sampler2d slope_Metal;
sampler2d slope_Rough;

//@ Shared Section:



 




//@ Gbuffer Frag Section:
float saturate(float x) {
	return clamp(x, 0, 1);
}

float remap(float value, float low1, float high1, float low2, float high2) {
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}


void main() {
	vec2 tUv = uv * ubo.flat_UvScale;
	vec2 sUv = uv * ubo.slope_UvScale;
	vec3 slope_normal = normalize(texture(slope_Normal, sUv).rgb);
	vec3 slope_albedo = texture(slope_Albedo, sUv).rgb;
	float slope_roughness = texture(slope_Rough, sUv).r;
	float slope_metal = texture(slope_Metal, sUv).r;
	
	vec3 flat_normal = normalize(texture(flat_Normal, tUv).rgb);
	vec3 flat_albedo = texture(flat_Albedo, tUv).rgb;
	float flat_roughness = texture(flat_Rough, tUv).r;
	float flat_metal = texture(flat_Metal, tUv).r;
	

  	vec4 worldNormal = vec4(normalize(TBN * vec3(0,0,1)), 1.f);
    float angle = dot(worldNormal.rgb, vec3(0, 1, 0)) * 2 / 3.1415;
    
	vec3 normal = flat_normal;

	float rough = 0;
	float metal = 0;
	rough = flat_roughness;
	metal = flat_metal;
	
	float slopeness = saturate(remap(pow(1 - angle, ubo.anglePow), ubo.angleLow, ubo.angleHigh, 0, 1));
	vec3 color = mix(flat_albedo, slope_albedo, slopeness);
	rough = mix(flat_roughness, slope_roughness, slopeness);
	normal = normalize(mix(flat_normal, slope_normal, slopeness));
	
				
	gPosition = vec4(fragPos, 1.f);
    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
    

	gAlbedoOpacity = vec4(color, 1.0);
	
    
//  	gAlbedoOpacity = vec4(percAngle);
	
	

	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	gSpecular = vec4(metal, rough, 0.f, 1.f);
	gEmissive = vec4(0, 0, 0.f, 1.f);
//	gEmissive = vec4(uv.x, uv.y, 0.f, 1.f);
}                                                                                        







//@ Depthmap Pass Section:

void main() {}
 












//@ Gbuffer Vert Section:
vec3 nrml;
vec3 tngt;
vec3 btng;
float g_samples[4]; 

vec3 g_pos[4];

 

vec3 OffsetPosition(vec2 uv) {
	float SAMPLE_DIST = 1 / max(ubo.quads, 1);
	float POS_OFFSET =  max(ubo.worldSize, 1) / max(ubo.quads, 1);

	g_samples[0] = texture(heightmap, vec2(uv.x + SAMPLE_DIST, uv.y)).r * ubo.HeightMultiplier;
	g_samples[1] = texture(heightmap, vec2(uv.x - SAMPLE_DIST, uv.y)).r * ubo.HeightMultiplier;
	g_samples[2] = texture(heightmap, vec2(uv.x, uv.y + SAMPLE_DIST)).r * ubo.HeightMultiplier;
	g_samples[3] = texture(heightmap, vec2(uv.x, uv.y - SAMPLE_DIST)).r * ubo.HeightMultiplier;
	
	g_pos[0] = vec3(POS_OFFSET, g_samples[0], 0);
	g_pos[1] = vec3(-POS_OFFSET, g_samples[1], 0);
	g_pos[2] = vec3(0, g_samples[2], POS_OFFSET);
	g_pos[3] = vec3(0, g_samples[3], -POS_OFFSET);
	
	tngt = normalize(g_pos[1] - g_pos[0]);
	btng = normalize(g_pos[3] - g_pos[2]);

	//btng = normalize(g_pos[1] - g_pos[0]);
//	tngt = normalize(g_pos[3] - g_pos[2]);

			
	return vec3(0, texture(heightmap, uv).r * ubo.HeightMultiplier, 0) * normal;
}

vec3 EditNormal(vec3 originalNormal) {
	nrml = cross(btng, tngt);
	return nrml;
}

vec3 EditTangent(vec3 originalTangent) { 
	return tngt;
}
 


 




