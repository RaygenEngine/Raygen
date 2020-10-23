#version 460
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

// TODO: test these (directly from svgf sample implementation)
float normalDistanceCos(vec3 n1, vec3 n2, float power)
{
	//return pow(max(0.0, dot(n1, n2)), 128.0);
	//return pow( saturate(dot(n1,n2)), power);
	return 1.0f;
}

float computeWeight(
	float depthCenter, float depthPixel, float phiDepth,
	vec3 normalCenter, vec3 normalPixel, float normPower, 
	float luminanceCenter, float luminancePixel, float phi){

	const float wNormal    = normalDistanceCos(normalCenter, normalPixel, normPower);
	const float wZ         = (phiDepth == 0) ? 0.0f : abs(depthCenter - depthPixel) / phiDepth;
	const float wLuminance = abs(luminanceCenter - luminancePixel) / phi;

	return exp(0.0 - max(wLuminance, 0.0) - max(wZ, 0.0)) * wNormal;
}

// computes a 3x3 gaussian blur of the variance, centered around
// the current pixel
float computeVarianceCenter(ivec2 iuv)
{
    float sum = 0.;

    const float kernel[2][2] = {
        { 1.0 / 4.0, 1.0 / 8.0  },
        { 1.0 / 8.0, 1.0 / 16.0 }
    };

    const int radius = 1;
    for (int yy = -radius; yy <= radius; yy++)
    {
        for (int xx = -radius; xx <= radius; xx++)
        {
            ivec2 sampleIuv = iuv + ivec2(xx, yy);
            float k = kernel[abs(xx)][abs(yy)];
			sum += imageLoad(svgfInput, sampleIuv).a * k;
        }
    }
    return sum;
}

bool IsReprojValid(ivec2 coord, float centerDrawId)
{
	vec4 thisUvDrawIndex = texelFetch(g_UVDrawIndexSampler, coord, 0);
	ivec2 iuv = ivec2(gl_FragCoord.xy);
	float a = texelFetch(g_SpecularSampler, coord, 0).a;
	float a2 = texelFetch(g_SpecularSampler, iuv, 0).a;
	
	if(abs(sqrt(a) - sqrt(a2)) > 0.1) return false;
	
	if (abs(thisUvDrawIndex.z - centerDrawId) >= 1) {
		return false;
	}

	return true;
}


void DebugRenderPasses();

void OutputColor(vec4 color);
void OutputColor(vec4 color, bool mixAlbedo);

bool IsInside(ivec2 p, ivec2 screenSize) {
	return p.x >= 0 && p.y >= 0 && p.x < screenSize.x && p.y < screenSize.y;
}

struct PixelData {
	vec4 color;
	vec3 normal;
	float depth;
	float luminance;
};



PixelData LoadPixelData(ivec2 iuv, ivec2 screenSize) {
	PixelData data;
	data.depth = texture(g_DepthSampler, uv).r;
	if (data.depth == 1) {
		return data;
	}
	data.color = imageLoad(svgfInput, iuv);
	data.luminance = luminance(data.color.rgb);
	
	vec2 uv = iuv / screenSize; 
	data.normal = texture(g_NormalSampler, uv).rgb;

	return data;
}

void main() {
	ivec2 iuv = ivec2(gl_FragCoord.xy);
	ivec2 screenSize = imageSize(svgfInput);
	vec4 color = imageLoad(svgfInput, iuv);

	if (totalIter == 0 && iteration >= totalIter - 1) {
		OutputColor(color);
		return;
	}

	int stepSize = 1 << iteration;

    const float epsVariance      = 1e-10;
    const float kernelWeights[3] = { 1.0, 2.0 / (3.0), 1.0 / 6.0 };
	const float centerDrawId = 	texelFetch(g_UVDrawIndexSampler, iuv, 0).z;


    const float var = computeVarianceCenter(iuv);

	const PixelData center = LoadPixelData(iuv, screenSize);

	// Skybox
    if (center.depth >= 1.f) {
    	OutputColor(color, false);
		return;
    }

	// phiColor: 10.0 default value
	const float phiColor            = 10.;
	const float phiNormal           = 128.;
    const float phiLIndirect = phiColor * sqrt(max(0.0, epsVariance + var));
    const float phiDepth     = max(center.depth, 1e-8) * stepSize;


 	// explicitly store/accumulate center pixel with weight 1 to prevent issues
    // with the edge-stopping functions
    const float centerMultiplier = 2.0f;
    float sumWIndirect = centerMultiplier;
    vec4  sumIndirect  = centerMultiplier * vec4(saturate(center.color.xyz), center.color.w);

    for (int yy = -2; yy <= 2; yy++)
    {
        for (int xx = -2; xx <= 2; xx++)
        {
            const ivec2 sampleIuv = iuv + ivec2(xx, yy) * stepSize;
            const bool inside = IsInside(sampleIuv, screenSize);

            const float kernel = kernelWeights[abs(xx)] * kernelWeights[abs(yy)];

            if (inside && (xx != 0 || yy != 0)) // skip center pixel, it is already accumulated
            {
            	if (IsReprojValid(sampleIuv, centerDrawId)) {
   		        	const PixelData pixel = LoadPixelData(sampleIuv, screenSize);
	
		            // compute the edge-stopping functions
		            const float w = computeWeight(
		                center.depth, pixel.depth, phiDepth * length(vec2(xx, yy)),
						center.normal, pixel.normal, phiNormal, 
		                center.luminance, pixel.luminance, phiLIndirect);
		
		            const float wIndirect = w * kernel;
		
		
		            sumWIndirect  += wIndirect;
		            sumIndirect   += vec4(wIndirect.xxx, wIndirect * wIndirect) * pixel.color;
                }
            }
        }
    }
	
	sumIndirect = vec4(sumIndirect / vec4(sumWIndirect.xxx, sumWIndirect * sumWIndirect));

	OutputColor(sumIndirect);
	//outColor = vec4(vec3(phiLIndirect) / 100., 1.f);
}                               

//
// 
//

void OutputColor(vec4 color, bool mixAlbedo) {
	ivec2 iuv = ivec2(gl_FragCoord.xy);
	if (iteration >= totalIter - 1) {
	    if (mixAlbedo) {
	    	vec4 colorSampleSpec = texelFetch(g_SpecularSampler, iuv, 0);
			vec4 colorSampleAlbe = texelFetch(g_AlbedoSampler, iuv, 0);
			float metallic = texelFetch(g_UVDrawIndexSampler, iuv, 0).a;

			vec4 p = max(mix(colorSampleAlbe, colorSampleSpec, metallic), vec4(0.1));
	    	
			color *= p;    	
	    	
	    }
   	    outColor = color;
	}
	else {
		imageStore(svgfOutput, iuv, color);
	}

	if (iteration == progressiveFeedbackIndex) {
		imageStore(progressiveResult, iuv, color);
	}
}

void OutputColor(vec4 color) {
	OutputColor(color, true);
}

void DebugRenderPasses() {
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













