#ifndef rt_indirect_glsl
#define rt_indirect_glsl

// META
// Expects pre declared variable prd before the inclusion of the file

layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

vec3 Contribution(vec3 throughput, vec3 nextOrigin, vec3 nextDirection, uint seed) {
	prd.radiance = vec3(0);
    prd.throughput = throughput;
	prd.depth = 1;
    prd.seed = seed;

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
	
	return throughput * prd.radiance;
}

vec3 TraceNext(vec3 throughput, vec3 L_shadingSpace, FsSpaceInfo fragSpace) {
    // TODO: Current bug: This seed needs to go back into the function
    // we currently take the same seed for p_spawn for both Indirect rays

    // RR termination
	float p_spawn = max(throughput.x, max(throughput.y, throughput.z));

	if(rand(fragSpace.seed) > p_spawn) {
		return vec3(0); 
	}
	
	vec3 weightedThroughput = throughput / p_spawn;
    
    outOnbSpace(fragSpace.orthoBasis, L_shadingSpace);
    return Contribution(weightedThroughput, fragSpace.worldPos, L_shadingSpace, fragSpace.seed); 
}

vec3 TraceIndirect(FsSpaceInfo fragSpace, FragBrdfInfo brdfInfo) {
    vec3 radiance = vec3(0.f);

    vec3 V = fragSpace.V;
    vec3 diffuseColor = brdfInfo.diffuseColor;
    vec3 f0 = brdfInfo.f0;
    float a = brdfInfo.a;

    float NoV = max(Ndot(V), BIAS);

    {
        vec2 u = rand2(fragSpace.seed); 
        vec3 L = cosineSampleHemisphere(u);

        float NoL = max(Ndot(L), BIAS); 

        vec3 H = normalize(V + L);
        float LoH = max(dot(L, H), BIAS);

        float pdf = NoL * INV_PI;
    
        vec3 brdf_d = DisneyDiffuse(NoL, NoV, LoH, a, diffuseColor);

        radiance += TraceNext(brdf_d * NoL / pdf, L, fragSpace);
    }

    // Glossy reflection
    {
        // WIP: test much lower alpha values for brdf (this is roughness ~= 0.3)
        if(a < 0.1) 
        {
            vec3 L = reflect(V);
            
            vec3 H = normalize(V + L);

            float NoL = max(Ndot(L), BIAS); 

            float NoH = max(Ndot(H), BIAS); 
            float LoH = max(dot(L, H), BIAS);

            float pdf = 1;


            // WIP: Dont sample brdf? 
            vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, a, f0);

            radiance += TraceNext(vec3(NoL), L, fragSpace);
        }
        else 
        {
            vec2 u = rand2(fragSpace.seed);
            vec3 H = importanceSampleGGX(u, a);

            vec3 L = reflect(-V, H);

            float NoL = max(Ndot(L), BIAS); 

            float NoH = max(Ndot(H), BIAS); 
            float LoH = max(dot(L, H), BIAS);

        
            float pdf = D_GGX(NoH, a) * NoH /  (4.0 * LoH);
            pdf = max(Ndot(H), BIAS); 

            vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, a, f0);

            radiance += TraceNext(brdf_r * NoL / pdf, L, fragSpace);
        }
    }

    return radiance;
}

#else
#error "RT Indirect glsl header should have no reason to be included twice (it declares descriptors). Check what you are trying to do"
#endif
