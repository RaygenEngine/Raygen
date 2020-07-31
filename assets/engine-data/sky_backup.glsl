// Raygen exported generated shader backup
//@ UBO Section:
col4 sunColor;
vec4 sunDirection;
col4 betaR;
col4 betaM;
float sunIntensity;
float planetRadius;
float atmosphereRadius;
float Hr;
float Hm;

ubo ubo;

//@ Shared Section:














//@ Gbuffer Frag Section:

void main() {
	vec3 normal = normalize(vec3(0.5, 0.5, 1.0) * 2.0 - 1.0);

    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
    gAlbedoOpacity = vec4(0.3f, 0.3f, 0.3f, 1.f);
	
	// r: metallic, g: roughness, b: reflectance, a: occlusion strength
	gSurface = vec4(0.f, 0.5f, 0.5f, 0.f);
	gEmissive = vec4(0.f, 0.f, 0.f, 1.f);
}                                                                                        














//@ Depthmap Pass Section:

void main() {}














//@ Gbuffer Vert Section:














//@ Unlit Frag Section:
#include "global.h"

#define USE_INSTANCE_FOR_ATMOSPHERE 0


void main() {

	vec3 sunColor = ubo.sunColor.rgb;
	float sunIntensity = ubo.sunIntensity;
	vec3 sunDirection = normalize(-ubo.sunDirection.xyz);     
    float earthRadius = 6360e3;       
    float atmosphereRadius = 6420e3; 
    // Thickness of the atmosphere if density was uniform (Hr)
    float Hr = 7994;                
     // Same as above but for Mie scattering (Hm) 
    float Hm = 1200;              
  


#if USE_INSTANCE_FOR_ATMOSPHERE == 1
	Hr = ubo.Hr;
	Hm = ubo.Hm;
	const vec3 betaR = ubo.betaR.xyz * 1e-4f;
    const vec3 betaM = ubo.betaM.xyz * 1e-4f; 
    earthRadius = ubo.planetRadius;
    atmosphereRadius = ubo.atmosphereRadius;
#else
	// Paper: https://hal.inria.fr/file/index/docid/288758/filename/article.pdf
    const vec3 betaR = vec3(5.8e-6f, 13.5e-6f, 33.1e-6f); 
    const vec3 betaM = vec3(21e-6f); 
#endif
	uint numSamples = 16; 
    uint numSamplesLight = 8; 
    
    vec3 orig = camera.position; 
    // camera ray dir
    vec3 dir = normalize(fragPos - orig); 
    
    // mu is the cosine of the angle between
    // the sun direction and the ray direction
    // CHECK: do we saturate here? 
    float mu = saturate(dot(dir, sunDirection));  
    float phaseR = 3.f / (16.f * PI) * (1 + mu * mu); 
   
    float g = 0.76f; 
    float phaseM = 3.f / (8.f * PI) * ((1.f - g * g) * (1.f + mu * mu)) / 
    ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f)); 

	// DANGER BIG FLOATS:
    float seabedToAtmoDst = atmosphereRadius - earthRadius;
    
    // real atmo intersection point
    vec3 end = seabedToAtmoDst * dir;
    float segmentLength = length(orig - end) / numSamples;
    
    vec3 sumR = vec3(0); 
    vec3 sumM = vec3(0); 
    
    float tCurrent = 0; 
    float opticalDepthR = 0; // rayleigh contribution
    float opticalDepthM = 0; // mie contribution
    
    // PERF: 
    for (uint i = 0; i < numSamples; ++i) {
     
        vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir; 
      
        float height = samplePosition.y; 
        
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength; 
        float hm = exp(-height / Hm) * segmentLength; 
        opticalDepthR += hr; 
        opticalDepthM += hm; 
        
        // light optical depth
        vec3 lightEnd = seabedToAtmoDst * sunDirection;
    	float segmentLengthLight = length(samplePosition - lightEnd) / numSamplesLight;
        
        float tCurrentLight = 0; 
        float opticalDepthLightR = 0;
        float opticalDepthLightM = 0;
 
        for (int j = 0; j < numSamplesLight; ++j) { 
            
            vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;      
            float heightLight = samplePositionLight.y; 
           
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight; 
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
             
            tCurrentLight += segmentLengthLight; 
        } 
        
        vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM *
                   1.1f * (opticalDepthM + opticalDepthLightM); 
        vec3 attenuation = vec3(exp(-tau.x), exp(-tau.y), exp(-tau.z)); 
        
        sumR += attenuation * hr; 
        sumM += attenuation * hm; 
         
        tCurrent += segmentLength; 
    } 
 
    outColor = vec4((sumR * betaR * phaseR + sumM * betaM * phaseM) * sunIntensity * sunColor, 1);
//    outColor = vec4(fragPos, 1) / 10000;
//    outColor = vec4(-normal, 1) / 50;
 } 
 








                                                                                                                                                                      
                                                                                                                                                                          



                                                                                                                                                                                                            

                                                                                                                                                                                                                    