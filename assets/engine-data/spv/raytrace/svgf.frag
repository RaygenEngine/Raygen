#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;



layout(set = 1, binding = 0, rgba32f) uniform image2D progressiveResult;
layout(set = 1, binding = 1, rgba32f) uniform image2D momentsBuffer;
layout(set = 1, binding = 2, rgba32f) uniform image2D svgfInput;
layout(set = 1, binding = 3, rgba32f) uniform image2D svgfOutput;

layout(push_constant) uniform PC {
	int iteration;
	int totalIter;
	int progressiveFeedbackIndex;
};

void main() {
	ivec2 iuv = ivec2(gl_FragCoord.xy);
	vec3 color = imageLoad(svgfInput, iuv).xyz;

	if ((iteration * 10) < iuv.x + 5
	 	&& ((iteration+1) * 10) > iuv.x ) {
			color += 
				iteration % 2 == 0 ? 
					vec3(0.1, 0, 0) : vec3(0, 0.1, 0);

	}

	if (iteration == totalIter - 1) {
	    outColor = vec4(color, 1.f); 
	}
	else {
		imageStore(svgfOutput, iuv, vec4(color, 1.));
	}
}                               


