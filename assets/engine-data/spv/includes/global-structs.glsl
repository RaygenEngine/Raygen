#ifndef global_structs_glsl
#define global_structs_glsl

struct Pointlight {                                                                                                                  
	vec3 position;                                                                                               
	float pad0;  
	
	vec3 color;                                                                                                  
	float pad3; 
	
	float intensity;                                                                                             
	float constantTerm;                                                                                          
	float linearTerm;                                                                                            
	float quadraticTerm;  
	
	float radius;                                                                                                
	int samples;                                                                                                 
	int hasShadow;                                                                                               
};

struct Camera {
	vec3 position;
	float pad0;

	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;

	float filmArea;
};

struct GltfMaterial {
	// factors
    vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metalnessFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;
	float baseReflectivity;

	float alphaCutoff;
	int alphaMode;

	bool doubleSided;
};

struct Dirlight {
	vec3 front;
	float pad0;

	mat4 viewProj;

	vec3 color;
	float pad3;

	float intensity;
	float maxShadowBias;
	int samples;
	float sampleInvSpread; 
	int hasShadow;    
};

struct Spotlight {
		vec3 position;
		float pad0;

		vec3 front;
		float pad1;

		// Lightmap
		mat4 viewProj;

		vec3 color;
		float pad3;

		float intensity;
		float near;
		float far;
		float outerCutOff;

		float innerCutOff;
		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float maxShadowBias;
		int samples;
		float sampleInvSpread;
		int hasShadow;    
};

struct Reflprobe {
	int lodCount;
	float radius;
	float irradianceFactor;
	float pad;
	vec3 position;
	float pad1;
};

struct Irragrid {
	int width;
	int height;
	int depth;
	int builtCount;

	vec3 firstPos;
	float distToAdjacent;
};

struct Quadlight {
	vec3 center;
	float pad0;
	vec3 normal;
	float pad1;
	vec3 right;
	float width;
	vec3 up;
	float height;

	vec3 color;
	float pad2;

	float intensity;
	float constantTerm;
	float linearTerm;
	float quadraticTerm;

	float cosAperture;

	float radius;

	int samples;
	int hasShadow;
};

#endif
