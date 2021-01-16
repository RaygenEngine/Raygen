#ifndef global_bsdfspace_glsl
#define global_bsdfspace_glsl

// math here are in bsdf space
// i.e. change basis of vectors using the shading normal of the surface

// n is the normal in surface space and is equal to vec3(0,0,1)
// w is the omega direction | must be normalized, otherwise math may break

// theta = angle between w and n (z-axis)
// sperical coordinates (theta, phi)

// cosTheta = n o w = (0,0,1) o w = w.z
float Ndot(vec3 w) { return w.z; }
float absNdot(vec3 w) { return abs(w.z); }
float CosTheta(vec3 w) { return w.z; }
float Cos2Theta(vec3 w) { return w.z * w.z; }
float AbsCosTheta(vec3 w) { return abs(w.z); }

// sin2Theta = 1 - cos2Theta
float Sin2Theta(vec3 w) { return max(0.f, 1.f - w.z * w.z); }
float SinTheta(vec3 w) { return sqrt(Sin2Theta(w)); }

// tanTheta = sinTheta / cosTheta
float TanTheta(vec3 w) { return SinTheta(w) / CosTheta(w); }
float Tan2Theta(vec3 w) { return Sin2Theta(w) / Cos2Theta(w); }
float AbsTanTheta(vec3 w) { return abs(SinTheta(w) / CosTheta(w)); }

// cosPhi = x / sinTheta
float CosPhi(vec3 w) 
{ 
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
}

float Cos2Phi(vec3 w) { return CosPhi(w) * CosPhi(w); }

// sinPhi = y / sinTheta
float SinPhi(vec3 w) 
{ 
    float sinTheta = SinTheta(w);
    return sinTheta == 0 ? 1 : clamp(w.y / sinTheta, -1, 1);
}

float Sin2Phi(vec3 w) { return SinPhi(w) * SinPhi(w); }

// DPhi can be found by zeroing the z coordinate of the two vectors to get 2D vectors 
// and then normalizing them. The dot product of these two vectors gives the cosine of the angle between them.
 
float CosDPhi(vec3 wa, vec3 wb) 
{
    return clamp((wa.x * wb.x + wa.y * wb.y) / 
            sqrt((wa.x * wa.x + wa.y * wa.y) *      
                 (wb.x * wb.x + wb.y * wb.y)), -1, 1);
}

bool sameHemisphere(vec3 w, vec3 wp) 
{
    return w.z * wp.z > 0;
}

// reflection on normal
// refl (wo, n) = -wo + 2 * dot(wo, n) * n =
//  = vec3(-wo.x, -wo.y, wo.z); note: opposite signs for consistency with glsl reflect
vec3 reflect(vec3 wo) 
{    
	return vec3(wo.x, wo.y, -wo.z);
}

vec3 refract(vec3 wo, float eta)
{
	float k = 1.0 - eta * eta * (1.0 - wo.z * wo.z);
    if (k < 0.0) {
        return vec3(0);
	}
	else {
        return vec3(eta * wo.x, eta * wo.y, -sqrt(k));
	}
}

#endif
