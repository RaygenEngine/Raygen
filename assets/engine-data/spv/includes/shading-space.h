// shading coordinate space
// scs

// n is the normal in shading space and is equal to vec3(0,0,1)
// w is the omega direction | must be normalized, otherwise math may break

// theta = angle between w and n (z-axis)


// sperical coordinates (theta, phi)

// cosTheta = n o w = (0,0,1) o w = w.z
float cosTheta(vec3 w) { return w.z; }
float cos2Theta(vec3 w) { return w.z * w.z; }

// sin2Theta = 1 - cos2Theta
float sin2Theta(vec3 w) { return max(0.f, 1.f - w.z * w.z); }
float sinTheta(vec3 w) { return sqrt(sin2Theta(w)); }

// tanTheta = sinTheta / cosTheta
float tanTheta(vec3 w) { return sinTheta(w) / cosTheta(w); }
float tan2Theta(vec3 w) { return sin2Theta(w) / cos2Theta(w); }

// cosPhi = x / sinTheta
float cosPhi(vec3 w) { 
    float sinTheta_ = sinTheta(w);
    return (sinTheta_ == 0) ? 1 : clamp(w.x / sinTheta_, -1, 1);
}

float cos2Phi(vec3 w) { return cosPhi(w) * cosPhi(w); }

// sinPhi = y / sinTheta
float sinPhi(vec3 w) 
{ 
    float sinTheta_ = sinTheta(w);
    return sinTheta_ == 0 ? 1 : clamp(w.y / sinTheta_, -1, 1);
}

float sin2Phi(vec3 w) { return sinPhi(w) * sinPhi(w); }

// DPhi can be found by zeroing the z coordinate of the two vectors to get 2D vectors 
// and then normalizing them. The dot product of these two vectors gives the cosine of the angle between them.
 
float cosDPhi(vec3 wa, vec3 wb) 
{
    return clamp((wa.x * wb.x + wa.y * wb.y) / 
            sqrt((wa.x * wa.x + wa.y * wa.y) *      
                 (wb.x * wb.x + wb.y * wb.y)), -1, 1);
}

// specular reflection
// refl (wo, n) = -wo + 2 * Dot(wo, n) * n =
//  = vec3(-wo.x, -wo.y, wo.z);
vec3 reflect(vec3 wo) 
{
    return vec3(-wo.x, -wo.y, wo.z);
}

float D_TrowbridgeReitzDistribution(vec3 wh, float a) 
{
    float alphax = a;
    float alphay = a;

    float tan2Theta_ = tan2Theta(wh);
    if (isinf(tan2Theta_)) return 0.f;
    float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    float e = (cos2Phi(wh) / (alphax * alphax) +
               sin2Phi(wh) / (alphay * alphay)) * tan2Theta_;
    return 1 / (PI * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
}

float L_TrowbridgeReitzDistribution(vec3 w, float a) 
{
    float alphax = a;
    float alphay = a;

    float absTanTheta = abs(tanTheta(w));
    if (isinf(absTanTheta)) return 0.f;

    float alpha = sqrt(cos2Phi(w) * alphax * alphax +
                       sin2Phi(w) * alphay * alphay);

    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;
}

float G1_TrowbridgeReitzDistribution(vec3 w, float a) 
{ 
    return 1 / (1 + L_TrowbridgeReitzDistribution(w, a)); 
}

float G_TrowbridgeReitzDistribution(vec3 wo, vec3 wi, float a) {
    return 1 / (1 + L_TrowbridgeReitzDistribution(wo, a) + L_TrowbridgeReitzDistribution(wi, a));
}

vec3 F_Schlick2(float NoV, vec3 f0) {
    float f = pow(1.0 - NoV, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 MicrofacetReflection(vec3 wo, vec3 wi, float a, vec3 f0)
{
    float cosThetaO = abs(cosTheta(wo)); // NoV
    float cosThetaI = abs(cosTheta(wi)); // NoL

    vec3 wh = wi + wo; // halfway vector

    // CHECK:
    //float LoH = dot(wi, wh);
    // vec3 f90 = vec3(0.5 + 2.0 * a * LoH * LoH);
 
 
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0.f);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0.f);

    wh = normalize(wh);
    vec3 F = F_Schlick2(dot(wi, wh), f0);
    return f0 * D_TrowbridgeReitzDistribution(wh, a) * G_TrowbridgeReitzDistribution(wo, wi, a) * F /
           (4 * cosThetaI * cosThetaO);
}

vec3 LambertianReflection(vec3 wo, vec3 wi, vec3 R) 
{
    return R * INV_PI;
}


bool sameHemisphere(vec3 w, vec3 wp) {
    return w.z * wp.z > 0;
}


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

float cosineHemispherePdf(float cosTheta_) 
{ 
    return cosTheta_ * INV_PI; 
}

vec3 uniformSampleCone(vec2 u, float cosThetaMax) 
{
    float cosTheta_ = (1.f - u.x) + u.x * cosThetaMax;
    float sinTheta_ = sqrt(1.f - cosTheta_ * cosTheta_);
    float phi = u.y * 2 * PI;
    return vec3(cos(phi) * sinTheta_, sin(phi) * sinTheta_,
                    cosTheta_);
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

vec3 toSurface(vec3 normal, vec3 tangent, vec3 binormal, vec3 v)
{
	return vec3(dot(v, tangent), dot(v, binormal), dot(v, normal));
}

vec3 toWorld(vec3 normal, vec3 tangent, vec3 binormal, vec3 v)
{
	return vec3(tangent.x * v.x + binormal.x * v.y + normal.x * v.z,
		        tangent.y * v.x + binormal.y * v.y + normal.y * v.z,
		        tangent.z * v.x + binormal.z * v.y + normal.z * v.z);
}

vec3 importanceSampleGGX2(vec2 Xi, float a) 
{
    const float phi = 2.0f * PI * Xi.x;
    // (aa-1) == (a-1)(a+1) produces better fp accuracy
    const float cosTheta2 = (1 - Xi.y) / (1 + (a + 1) * ((a - 1) * Xi.y));
    const float cosTheta = sqrt(cosTheta2);
    const float sinTheta = sqrt(1 - cosTheta2);

    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	return normalize(H);
}