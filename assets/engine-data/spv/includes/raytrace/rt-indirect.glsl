#ifndef rt_indirect_glsl
#define rt_indirect_glsl

// META:
// Expects pre declared variable prd before the inclusion of the file
	
#ifndef TEST_CUBE
    layout(set = 3, binding = 0) 
#else
    layout(set = 1, binding = 0) 
#endif
    uniform accelerationStructureEXT topLevelAs;

// Handle just radiance here (no throughput). 
// This function should be able to  be used without throughput (eg: a debug ray directly from the camera that invokes hit shaders)
vec3 RadianceOfRay(vec3 nextOrigin, vec3 nextDirection) {
	prd.radiance = vec3(0);

    // PERF: any ideas?
    if(any(isnan(nextDirection)) || any(isinf(nextDirection))){
       return vec3(0.0);
    }

	prd.depth += 1;

    uint  rayFlags = gl_RayFlagsOpaqueEXT | gl_RayFlagsCullFrontFacingTrianglesEXT;
    float tMin     = 0.001;
    float tMax     = 10000.0;

	// trace ray
	traceRayEXT(topLevelAs,     // acceleration structure
				rayFlags,       // rayFlags
				0xFF,           // cullMask
				0,              // sbtRecordOffset
				0,              // sbtRecordStride
				0,              // missIndex
				nextOrigin,     // ray origin
				tMin,           // ray min range
				nextDirection,  // ray direction
				tMax,           // ray max range
				0               // payload (location = 0)
	);
	
    prd.depth -= 1;
	return prd.radiance;
}

// Handle Termination and throughput in this function, handle radiance in the above
vec3 TraceNext(vec3 thisRayThroughput, vec3 L_shadingSpace, FsSpaceInfo fragSpace) {

    vec3 prevCumulativeThroughput = prd.accumThroughput;
    // RR termination
    // TODO: Use perceived luminace of throughput for termination (instead of max)

	float p_spawn = max(max(thisRayThroughput * prevCumulativeThroughput), 0.1);
	if(rand(prd.seed) > p_spawn) {
		return vec3(0); 
	}

    thisRayThroughput /= p_spawn; 
    
    // Update accum throughput for the remaining of the recursion
    prd.accumThroughput = prevCumulativeThroughput * thisRayThroughput;

    outOnbSpace(fragSpace.orthoBasis, L_shadingSpace); // NOTE: not shading space after the function, make onb return values for better names
    vec3 result = thisRayThroughput * RadianceOfRay(fragSpace.worldPos, L_shadingSpace); 

    // Restore accum throughput
    prd.accumThroughput = prevCumulativeThroughput;
    return result;
}

//#define RNG_BRDF_X(bounce)                (4 + 4 + 9 * bounce)
//#define RNG_BRDF_Y(bounce)                (4 + 5 + 9 * bounce)
//
//float get_rng(uint idx, uint rng_seed)
//{
//	//uvec3 p = uvec3(rng_seed, rng_seed >> 10, rng_seed >> 20);
//	//p.z = (p.z + idx);
//	//p &= uvec3(BLUE_NOISE_RES - 1, BLUE_NOISE_RES - 1, NUM_BLUE_NOISE_TEX - 1); // array impl for temporal (z)
//
//	//return min(texelFetch(TEX_BLUE_NOISE, ivec3(p), 0).r, 0.9999999999999);
//	//return fract(vec2(get_rng_uint(idx)) / vec2(0xffffffffu));
//	uvec2 p = uvec2(rng_seed, rng_seed >> 10);
//	p &= uvec2(470 - 1);
//	
//	return texture(blueNoiseSampler, vec2(p)).r;
//}
 

vec3 TraceIndirect(FsSpaceInfo fragSpace, FragBrdfInfo brdfInfo) {
    vec3 radiance = vec3(0.f);

    vec3 V = fragSpace.V;
    vec3 albedo = brdfInfo.albedo;
    vec3 f0 = brdfInfo.f0;
    float a = brdfInfo.a;

    float NoV = max(Ndot(V), BIAS);

    float p_specular = 0.5;

    // Glossy reflection
    if(rand(prd.seed) > p_specular)
    {
        // Ideally specular
        vec2 u = rand2(prd.seed);
        vec3 H = importanceSampleGGX(u, a);

        vec3 L = reflect(-V, H);

        float NoL = max(Ndot(L), BIAS); 

        float NoH = max(Ndot(H), BIAS); 
        float LoH = max(dot(L, H), BIAS);

        
        float pdf = D_GGX(NoH, a) * NoH /  (4.0 * LoH);
        pdf = max(pdf, BIAS); // CHECK: pbr-book stops tracing if pdf == 0

        if(a < 0.0001){
            // SMATH: H is wrong here
            L = reflect(-V);
            NoL = max(Ndot(L), BIAS); 
           
            NoH = max(Ndot(H), BIAS); 
            LoH = max(dot(L, H), BIAS);
            pdf = 1.0;
        }   

        vec3 ks = F_Schlick(LoH, f0);

        vec3 brdf_r = SpecularTerm(NoV, NoL, NoH, a, ks) / p_specular;

        radiance += TraceNext(brdf_r * NoL / pdf, L, fragSpace);
    }

    // Diffuse reflection
    else
    {
        vec2 u = rand2(prd.seed); 
        vec3 L = cosineSampleHemisphere(u);

        float NoL = max(Ndot(L), BIAS); 

        vec3 H = normalize(V + L);
        float LoH = max(dot(L, H), BIAS);

        float pdf = NoL * INV_PI;

        vec3 kd = 1 - F_Schlick(LoH, f0);

        vec3 brdf_d = DiffuseTerm(NoL, NoV, LoH, a, albedo, kd) / (1 - p_specular);

        radiance += TraceNext(brdf_d * NoL / pdf, L, fragSpace);
    }

    return radiance;
}

#else
#error "RT Indirect glsl header should have no reason to be included twice (it declares descriptors). Check if what you are trying to do is correct."
#endif
