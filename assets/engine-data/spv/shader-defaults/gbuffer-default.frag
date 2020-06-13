#version 450
#extension GL_ARB_separate_shader_objects : enable

// out

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
// rgb: albedo, a: opacity
layout (location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec3 fragPos; 
	vec2 uv;
	mat3 TBN;
};

// uniforms

void main() {
	vec3 normal = normalize(vec3(0.5, 0.5, 1.0) * 2.0 - 1.0);
	// opacity set from above

    // position
    gPosition = vec4(fragPos, 1.f);
	
    // normal (with normal mapping)
    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
	
    // albedo opacity
    gAlbedoOpacity = vec4(0.3f, 0.3f, 0.3f, 1.f);
	
	// spec params
	gSpecular = vec4(0.f, 0.5f, 0.f, 0.f);
	
	// emissive
	gEmissive = vec4(0.f, 0.f, 0.f, 1.f);
}                                                                                        

