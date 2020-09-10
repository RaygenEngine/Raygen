#ifndef rt_indirect_glsl
#define rt_indirect_glsl

// META:
// Expects pre declared variable prd before the inclusion of the file

layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

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

vec3 TraceIndirect(FsSpaceInfo fragSpace, FragBrdfInfo brdfInfo) {
    vec3 radiance = vec3(0.f);

    vec3 V = fragSpace.V;
    vec3 diffuseColor = brdfInfo.diffuseColor;
    vec3 f0 = brdfInfo.f0;
    float a = brdfInfo.a;

    float NoV = max(Ndot(V), BIAS);

    // Diffuse reflection
    {
        vec2 u = rand2(prd.seed); 
        vec3 L = cosineSampleHemisphere(u);

        float NoL = max(Ndot(L), BIAS); 

        vec3 H = normalize(V + L);
        float LoH = max(dot(L, H), BIAS);

        float pdf = NoL * INV_PI;
    
        // SMATH: missing Kd
        vec3 brdf_d = DisneyDiffuse(NoL, NoV, LoH, a, diffuseColor);

        radiance += TraceNext(brdf_d * NoL / pdf, L, fragSpace);
    }

    // Glossy reflection
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
            L = reflect(-V);
            NoL = max(Ndot(L), BIAS); 
           
            NoH = max(Ndot(H), BIAS); 
            LoH = max(dot(L, H), BIAS);
            pdf = 1.0;
        }   

        vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, a, f0);

        radiance += TraceNext(brdf_r * NoL / pdf, L, fragSpace);
    }

    return radiance;
}

#else
#error "RT Indirect glsl header should have no reason to be included twice (it declares descriptors). Check if what you are trying to do is correct."
#endif
