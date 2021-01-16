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
	float sign = csign(n.z); 
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
	// normal and bitangent are orthogonal and normalized therefore tangent is normalized
	res.tangent  = cross(res.bitangent, res.normal); 

	return res;
}

vec3 toOnbSpace(Onb orthoBasis, vec3 v)
{
	return vec3(dot(v, orthoBasis.tangent), 
	            dot(v, orthoBasis.bitangent), 
			    dot(v, orthoBasis.normal)); 
}


vec3 outOnbSpace(Onb orthoBasis, vec3 v)
{
	return vec3(orthoBasis.tangent.x * v.x + orthoBasis.bitangent.x * v.y + orthoBasis.normal.x * v.z,
			    orthoBasis.tangent.y * v.x + orthoBasis.bitangent.y * v.y + orthoBasis.normal.y * v.z,
			    orthoBasis.tangent.z * v.x + orthoBasis.bitangent.z * v.y + orthoBasis.normal.z * v.z);
}

#endif
