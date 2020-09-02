// shading coordinate space
// scs

#ifndef shading_space_h
#define shading_space_h

// n is the normal in shading space and is equal to vec3(0,0,1)
// w is the omega direction | must be normalized, otherwise math may break

// theta = angle between w and n (z-axis)


// sperical coordinates (theta, phi)

// cosTheta = n o w = (0,0,1) o w = w.z
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
// refl (wo, n) = -wo + 2 * Dot(wo, n) * n =
//  = vec3(-wo.x, -wo.y, wo.z);
vec3 reflect(vec3 wo) 
{
    return vec3(-wo.x, -wo.y, wo.z);
}

float D_TrowbridgeReitzDistribution(vec3 wh, float alpha_x, float alpha_y) 
{
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta)) return 0.f;
    float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    float e = (Cos2Phi(wh) / (alpha_x * alpha_x) +
               Sin2Phi(wh) / (alpha_y * alpha_y)) * tan2Theta;
    return 1 / (PI * alpha_x * alpha_y * cos4Theta * (1 + e) * (1 + e));
}

float L_TrowbridgeReitzDistribution(vec3 w, float alpha_x, float alpha_y) 
{
    float absTanTheta = AbsTanTheta(w);
    if (isinf(absTanTheta)) return 0.f;

    float alpha = sqrt(Cos2Phi(w) * alpha_x * alpha_x +
                       Sin2Phi(w) * alpha_y * alpha_y);

    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;
}

float G1_TrowbridgeReitzDistribution(vec3 w, float alpha_x, float alpha_y) 
{ 
    return 1 / (1 + L_TrowbridgeReitzDistribution(w, alpha_x, alpha_y)); 
}

float G_TrowbridgeReitzDistribution(vec3 wo, vec3 wi, float alpha_x, float alpha_y) {
    return 1 / (1 + L_TrowbridgeReitzDistribution(wo, alpha_x, alpha_y) + L_TrowbridgeReitzDistribution(wi, alpha_x, alpha_y));
}

vec3 F_Schlick2(float NoV, vec3 f0) {
    float f = pow(1.0 - NoV, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 F_Schlick2(float NoV, vec3 f0, vec3 f90) {
    return f0 + (f90 - f0) * pow(1.0 - NoV, 5.0);
}

vec3 MicrofacetReflection(vec3 wo, vec3 wi, float alpha_x, float alpha_y, vec3 f0)
{
    float cosThetaO = AbsCosTheta(wo); // NoV
    float cosThetaI = AbsCosTheta(wi); // NoL

    vec3 wh = normalize(wi + wo); // halfway vector

    // CHECK:
    float LoH = dot(wi, wh);
    vec3 f90 = vec3(0.5 + 2.0 * alpha_x * alpha_y * LoH * LoH);
 
 
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0.f);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0.f);

    // CHECK
    vec3 F = F_Schlick2(dot(wi, wh), f0, f90);


    // SMATH:
    return f0 * D_TrowbridgeReitzDistribution(wh, alpha_x, alpha_y) * G_TrowbridgeReitzDistribution(wo, wi, alpha_x, alpha_y) * F /
           (4 * cosThetaI * cosThetaO);
}

vec3 LambertianReflection(vec3 wo, vec3 wi, vec3 R) 
{
    return R * INV_PI;
}

void TrowbridgeReitzSample11(float cosTheta, float U1, float U2,
                                    inout float slope_x, inout float slope_y) 
{
    // special case (normal incidence)
    if (cosTheta > .9999) {
         float r = sqrt(U1 / (1 - U1));
         float phi = 6.28318530718 * U2;
         slope_x = r * cos(phi);
         slope_y = r * sin(phi);
         return;
    }

    float sinTheta = sqrt(max(0.f, 1.f - cosTheta * cosTheta));
    float tanTheta = sinTheta / cosTheta;
    float a = 1.f / tanTheta;
    float G1 = 2.f / (1.f + sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    float A = 2.f * U1 / G1 - 1.f;
    float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10) tmp = 1e10;
    float B = tanTheta;
    float D = sqrt(max((B * B * tmp * tmp - (A * A - B * B) * tmp), 0.f));
    float slope_x_1 = B * tmp - D;
    float slope_x_2 = B * tmp + D;
    slope_x = (A < 0.f || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    float S;
    if (U2 > 0.5f) {
        S = 1.f;
        U2 = 2.f * (U2 - .5f);
    } else {
        S = -1.f;
        U2 = 2.f * (.5f - U2);
    }
    float z =
        (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
        (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    slope_y = S * z * sqrt(1.f + slope_x * slope_x);

    //CHECK(!std::isinf(*slope_y));
    //CHECK(!std::isnan(*slope_y));
}

vec3 TrowbridgeReitzSample(vec3 wi, float alpha_x, float alpha_y, float U1, float U2) 
{
    // 1. stretch wi
    vec3 wiStretched = normalize(vec3(alpha_x * wi.x, alpha_y * wi.y, wi.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    float slope_x, slope_y;
    TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2, slope_x, slope_y);

    // 3. rotate
    float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
    slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = alpha_x * slope_x;
    slope_y = alpha_y * slope_y;

    // 5. compute normal
    return normalize(vec3(-slope_x, -slope_y, 1.f));
}

vec3 TrowbridgeReitzDistribution_Sample_wh(vec3 wo, vec2 u, float alpha_x, float alpha_y) 
{
    // vec3 wh;
    // if (!sampleVisibleArea) {
        // Float cosTheta = 0, phi = (2 * Pi) * u[1];
        // if (alphax == alphay) {
            // Float tanTheta2 = alphax * alphax * u[0] / (1.0f - u[0]);
            // cosTheta = 1 / std::sqrt(1 + tanTheta2);
        // } else {
            // phi =
                // std::atan(alphay / alphax * std::tan(2 * Pi * u[1] + .5f * Pi));
            // if (u[1] > .5f) phi += Pi;
            // Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            // const Float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
            // const Float alpha2 =
                // 1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            // Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
            // cosTheta = 1 / std::sqrt(1 + tanTheta2);
        // }
        // Float sinTheta =
            // std::sqrt(std::max((Float)0., (Float)1. - cosTheta * cosTheta));
        // wh = SphericalDirection(sinTheta, cosTheta, phi);
        // if (!SameHemisphere(wo, wh)) wh = -wh;
    // } else {
        bool flip = wo.z < 0;
        vec3 wh = TrowbridgeReitzSample(flip ? -wo : wo, alpha_x, alpha_y, u.x, u.y);
        if (flip) wh = -wh;
    //}
    return wh;
}


float TrowbridgeReitzSamplePdf(vec3 wo, vec3 wh, float alpha_x, float alpha_y) 
{
    //if (sampleVisibleArea)
        return D_TrowbridgeReitzDistribution(wh, alpha_x, alpha_y) * G1_TrowbridgeReitzDistribution(wo, alpha_x, alpha_y) * abs(dot(wo, wh)) / AbsCosTheta(wo);
    //else
    //return D_TrowbridgeReitzDistribution(wh, alpha_x, alpha_y) * AbsCosTheta(wh);
}

#endif