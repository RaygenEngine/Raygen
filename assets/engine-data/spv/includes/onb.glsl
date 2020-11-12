#ifndef onb_glsl
#define onb_glsl

// normal - z axis, normal should be normalized
// tangent - x axis
// bitangent - y axis 
// SMATH: ensure somehow that to and out of OnbSpace remain normalized
struct Onb
{
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
};

// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
// SMATH: is the Onb matrix orthogonal?
Onb branchlessOnb(vec3 n)
{
	// n.z >= 0.0 ? 1.0 : -1.0;
	float sign = sign(n.z) + (1.0 - abs(sign(n.z))); 
	float a = -1.0f / (sign + n.z); 
	float b = n.x * n.y * a; 

	Onb res;
	res.normal = n;
	res.tangent = vec3(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
	res.bitangent = vec3(b, sign + n.y * n.y * a, -n.y);

	return res;
}

Onb computeOnb(vec3 n)
{
	Onb res;

	if(abs(n.x) > abs(n.z))
	{
		res.bitangent.x = -n.y;
		res.bitangent.y = n.x;
		res.bitangent.z = 0;
	}
	else
	{
		res.bitangent.x = 0;
		res.bitangent.y = -n.z;
		res.bitangent.z = n.y;
	}

	res.normal = n;
	res.bitangent = normalize(res.bitangent);
	// normal and bitangent are orthogonal therefore tangent is normalized
	res.tangent  = cross(res.bitangent, res.normal); 

	return res;
}

void toOnbSpace(Onb orthoBasis, inout vec3 v)
{
	v = vec3(dot(v, orthoBasis.tangent), 
	         dot(v, orthoBasis.bitangent), 
			 dot(v, orthoBasis.normal)); 
}

vec3 toOnbSpaceReturn(Onb orthoBasis, vec3 v)
{
	return vec3(dot(v, orthoBasis.tangent), 
	         dot(v, orthoBasis.bitangent), 
			 dot(v, orthoBasis.normal)); 
}

void outOnbSpace(Onb orthoBasis, inout vec3 v)
{
	v = vec3(orthoBasis.tangent.x * v.x + orthoBasis.bitangent.x * v.y + orthoBasis.normal.x * v.z,
			 orthoBasis.tangent.y * v.x + orthoBasis.bitangent.y * v.y + orthoBasis.normal.y * v.z,
			 orthoBasis.tangent.z * v.x + orthoBasis.bitangent.z * v.y + orthoBasis.normal.z * v.z);
}

// math here are in onb space 
// i.e. change basis of vectors using the normal of the surface

// n is the normal in bsdf space and is equal to vec3(0,0,1)
// w is the omega direction | must be normalized, otherwise math may break

// theta = angle between w and n (z-axis)
// sperical coordinates (theta, phi)

// cosTheta = n o w = (0,0,1) o w = w.z
float Ndot(vec3 w) { return w.z; }
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

vec3 importanceSampleGGX(vec2 u, float a) 
{
    float phi = 2.0f * PI * u.x;
    // (aa-1) == (a-1)(a+1) produces better fp accuracy
    float cosTheta2 = (1 - u.y) / (1 + (a + 1) * ((a - 1) * u.y));
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1 - cosTheta2);

    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}


#endif