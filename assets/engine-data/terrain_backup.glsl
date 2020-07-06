// Raygen exported generated shader backup
//@ UBO Section:
float HeightMultiplier;
float flat_UvScale;
float sdist;
float poffset;

ubo ubo;

sampler2d heightmap;
sampler2d flat_Albedo;
sampler2d flat_Normal;
sampler2d flat_MetalRough;

//@ Shared Section:


 



 


 

//@ Gbuffer Frag Section:

void main() {
	vec3 normal = normalize(vec3(0.5, 0.5, 1.0) * 2.0 - 1.0);

    gPosition = vec4(fragPos, 1.f);
    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
    gAlbedoOpacity = vec4(0.1);

	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	gSpecular = vec4(0.f, 0.5f, 0.f, 0.f);
	gEmissive = vec4(0.f, 0.f, 0.f, 1.f);
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
	float SAMPLE_DIST = ubo.sdist / 1000.f;
	float POS_OFFSET = ubo.poffset;

	g_samples[0] = texture(heightmap, vec2(uv.x + SAMPLE_DIST, uv.y + SAMPLE_DIST)).r * ubo.HeightMultiplier;
	g_samples[1] = texture(heightmap, vec2(uv.x + SAMPLE_DIST, uv.y - SAMPLE_DIST)).r * ubo.HeightMultiplier;
	g_samples[2] = texture(heightmap, vec2(uv.x - SAMPLE_DIST, uv.y + SAMPLE_DIST)).r * ubo.HeightMultiplier;
	g_samples[3] = texture(heightmap, vec2(uv.x - SAMPLE_DIST, uv.y - SAMPLE_DIST)).r * ubo.HeightMultiplier;
	
	g_pos[0] = vec3(POS_OFFSET, g_samples[0], POS_OFFSET);
	g_pos[1] = vec3(POS_OFFSET, g_samples[1], -POS_OFFSET);
	g_pos[2] = vec3(-POS_OFFSET, g_samples[2], POS_OFFSET);
	g_pos[3] = vec3(-POS_OFFSET, g_samples[3], -POS_OFFSET);
	
	tngt = normalize(g_pos[2] - g_pos[0]);
	btng = normalize(g_pos[3] - g_pos[0]);
	return vec3(0, texture(heightmap, uv).r * ubo.HeightMultiplier, 0) * normal;
}

vec3 EditNormal(vec3 originalNormal) {
	nrml = cross(btng, tngt);
	return nrml;
}

vec3 EditTangent(vec3 originalTangent) {
	return tngt;
}
 


