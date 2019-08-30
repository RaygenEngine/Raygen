#version 460 core
out vec4 out_color;
  
in vec3 FragPos;  
in vec3 Normal;
in vec2 UV;

uniform int mode;

uniform vec3 viewPos;

uniform sampler2D baseColorSampler;
uniform sampler2D metallicRoughnessSampler;
//uniform sampler2D normalSampler;
//uniform sampler2D occlusionSampler;
//uniform sampler2D emissiveSampler;

#define M_1_PIf 0.318309886183790671538f
#define M_PIf 3.14159265358979323846f

// very basic unoptimized surface test shader
void main()
{

	// TODO render to FBO, sample depth texture, show hdr skymap at far
	/*vec3 dir = normalize(FragPos - viewPos);

	//float theta = atan(dir.x, dir.z);
	//float phi = M_PIf * 0.5f - acos(dir.y);
	//float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	//float v = 0.5f * (1.0f + sin(phi));
	//if(depth)
	//out_color = vec4(vec3(texture(skyHDRSampler, vec2(u,v))), 1.0);*/

	const vec4 albedo = texture(baseColorSampler, UV);
	//const vec4 emission = texture(emissionSampler, UV);
	//const vec4 bump = texture(bumpSampler, UV);
	//const vec4 specular_parameters = texture(specularParametersSampler, UV);


	switch (mode)
	{
		case 0: // albedo
			out_color = vec4(vec3(albedo), 1.0);
			break;

		case 5: // world normal
			out_color = vec4(Normal, 1.0);
			break;

		case 8: // uv
			out_color = vec4(UV, 0.0, 1.0);
			break;
	}
} 