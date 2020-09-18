#ifndef global_shading_glsl
#define global_shading_glsl
#include "onb.glsl"
// Global Include for all shaders that perform any light / shading related stuff.
// Contains structures for standard spaces + aggregation structures for incidents to be used as parameters
// in other light header functions (Brdfs, Light Headers, etc)


// All surface information required to perform "standard" brdf.
// (We could have more of these for: 1) btdf 2) different brdf implementations eg: cloth, clear-coating)
struct FragBrdfInfo {
	vec3 albedo; // albedo

	vec3 f0; // specularColor 
	
	float a; // roughness^2
};


// Fragment Shading Space Info
// Contains space and view data for a specific "fragment" in the world as well as a seed (for utility)
// (Encapsulates all non material specific information about the "fragment")
// This struct should be used for all lighting functions
struct FsSpaceInfo {
	Onb orthoBasis;
	vec3 V; // In shading space
	vec3 worldPos;
};

// Holds all required space data to solve an incidence: 
// orthoBasis, 
// [V, L] in shading space 
// world Position (to be able to support additional ray tracing bounces)
struct IncidentSpaceInfo {
    FsSpaceInfo fragSpace;
    vec3 L; // In Shading Space

    // Cached Stuff:
    float NoL;
    float NoV;
    float NoH;
    float LoH;
};


FsSpaceInfo GetFragSpace_World(vec3 worldNormal, vec3 worldPos, vec3 worldViewPos) {
	FsSpaceInfo fragSpace;
	
	fragSpace.orthoBasis = branchlessOnb(worldNormal);
	fragSpace.worldPos = worldPos;
	fragSpace.V = toOnbSpaceReturn(fragSpace.orthoBasis, normalize(worldViewPos - worldPos));
	
	return fragSpace;
}


/*
WIP: 
void PopulateIncidentCache(inout IncidentSpaceInfo incidenceSpace) {
    
}

IncidentSpaceInfo MakeIncidence_World(FsSpaceInfo fragSpace, vec3 woldLightPos) {


    PopulateCache(..);
}

IncidentSpaceInfo MakeIncidence_Shading(FsSpaceInfo fragSpace, vec3 shadingLightDir) {

    
    PopulateCache(..); 
}
*/

#endif