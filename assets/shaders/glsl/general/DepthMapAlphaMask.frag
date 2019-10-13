#version 460 core

in Data
{ 
	vec2 text_coord[2];
} dataIn;

uniform bool mask;
uniform float alpha_cutoff;

uniform vec4 base_color_factor;
uniform int base_color_texcoord_index;

layout(binding=0) uniform sampler2D baseColorSampler;

void main()
{             
	vec4 sampled_base_color = texture(baseColorSampler, dataIn.text_coord[base_color_texcoord_index]);

	float opacity = sampled_base_color.a * base_color_factor.a;

	// mask mode and cutoff
	if(mask && opacity < alpha_cutoff)
		discard;
}  