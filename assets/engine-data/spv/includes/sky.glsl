#ifndef sky_glsl
#define sky_glsl

vec3 GetSkyColor(vec3 orig, vec3 dir)
{
	// PERF: this should be baked into a cubemap?
	vec3 sunColor = vec3(1, 1, 1);
	float sunIntensity = 40;
	vec3 sunDirection = normalize(-vec3(-1, -1, 0));
	float earthRadius = 6360e3;
	float atmosphereRadius = 6420e3;
	// Thickness of the atmosphere if density was uniform (Hr)
	float Hr = 7994;
	// Same as above but for Mie scattering (Hm)
	float Hm = 1200;

	// Paper: https://hal.inria.fr/file/index/docid/288758/filename/article.pdf
	const vec3 betaR = vec3(5.8e-6f, 13.5e-6f, 33.1e-6f);
	const vec3 betaM = vec3(21e-6f);

	uint numSamples = 16;
	uint numSamplesLight = 8;

	// mu is the cosine of the angle between
	// the sun direction and the ray direction
	// SMATH: do we saturate here?
	float mu = saturate(dot(dir, sunDirection));
	float phaseR = 3.f / (16.f * PI) * (1 + mu * mu);

	float g = 0.76f;
	float phaseM = 3.f / (8.f * PI) * ((1.f - g * g) * (1.f + mu * mu))
				   / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));

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

		vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
		vec3 attenuation = vec3(exp(-tau.x), exp(-tau.y), exp(-tau.z));

		sumR += attenuation * hr;
		sumM += attenuation * hm;

		tCurrent += segmentLength;
	}

	return vec3(sumR * betaR * phaseR + sumM * betaM * phaseM) * sunIntensity * sunColor;
}

vec3 GetSkyColor(vec3 direction)
{
	return GetSkyColor(vec3(0), direction);
}

#endif
