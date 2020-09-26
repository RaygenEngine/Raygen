#ifndef rt_callableMat_glsl
#define rt_callableMat_glsl

// META:
// Expects pre declared struct "Material" before the inclusion of the file

//layout(buffer_reference, std430) buffer MaterialBufRef { Material m; };

//struct CallableMatInOut
//{
//	//MaterialBufRef materialUbo; // Incoming Buffer reference
//	vec2 uv; // Incoming UV
//	
//	FragBrdfInfo brdfInfo;
//	
//	vec3 emissive;
//	vec3 localNormal;
//};

#else
#error "RT CallableMat glsl header should have no reason to be included twice. Check if what you are trying to do is correct."
#endif
