#ifndef sampling_h
#define sampling_h

vec3 uniformSampleHemisphere(vec2 u) 
{
    float z = u.x;
    float r = sqrt(max(0.f, 1.f - z * z));
    float phi = 2 * PI * u.y;
    return vec3(r * cos(phi), r * sin(phi), z);
}

float uniformHemispherePdf() 
{
    return INV_2PI;
}

vec3 uniformSampleSphere(vec2 u) 
{
    float z = 1 - 2 * u.x;
    float r = sqrt(max(0.f, 1.f - z * z));
    float phi = 2 * PI * u.y;
    return vec3(r * cos(phi), r * sin(phi), z);
}
 
float uniformSpherePdf() 
{
    return INV_4PI;
}

vec2 uniformSampleDisk(vec2 u) 
{
    float r = sqrt(u.x);
    float theta = 2 * PI * u.y;
    return vec2(r * cos(theta), r * sin(theta));
}

vec2 concentricSampleDisk(vec2 u)  
{
    vec2 uOffset = 2.f * u - vec2(1.f, 1.f);

    if (uOffset.x == 0 && uOffset.y == 0) return vec2(0, 0);

    float theta, r;
    if (abs(uOffset.x) > abs(uOffset.y)) {
        r = uOffset.x;
        theta = PI_OVER4 * (uOffset.y / uOffset.x);
    } else {
        r = uOffset.y;
        theta = PI_OVER2 - PI_OVER4 * (uOffset.x / uOffset.y);
    }
    return r * vec2(cos(theta), sin(theta));
}

vec3 cosineSampleHemisphere(vec2 u)   
{
    vec2 d = concentricSampleDisk(u);
    float z = sqrt(max(0.f, 1 - d.x * d.x - d.y * d.y));
    return vec3(d.x, d.y, z);
}

// TODO: not used fix
//float cosineHemispherePdf(vec3 wo, vec3 wi) 
//{
//     return sameHemisphere(wo, wi) ? AbsCosTheta(wi) * INV_PI : 0;
//}

vec3 uniformSampleCone(vec2 u, float cosThetaMax) 
{
    float cosTheta = (1.f - u.x) + u.x * cosThetaMax;
    float sinTheta = sqrt(1.f - cosTheta * cosTheta);
    float phi = u.y * 2 * PI;
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float uniformConePdf(float cosThetaMax) {
    return 1 / (2 * PI * (1 - cosThetaMax));
}

vec2 uniformSampleTriangle(vec2 u) 
{
    float su0 = sqrt(u.x);
    return vec2(1 - su0, u.y * su0);
}

// pdf = 1 / area of triangle

#endif