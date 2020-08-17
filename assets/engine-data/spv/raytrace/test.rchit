#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require

#include "global.h"

struct hitPayload
{
  vec4 hitValue;
};

//layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAs;

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {

	int matId = gl_InstanceID % 4;

	if (matId == 0){
		prd.hitValue = vec4(0.1,0,0,1);
	}
	else if (matId == 1){
		prd.hitValue = vec4(0.0,0.1,0,1);
	}
	else if (matId == 2){
		prd.hitValue = vec4(0.1,0.1,0,1);
	}
	else if (matId == 3){
		prd.hitValue = vec4(0.1,0.1,0,1);
	}
}



