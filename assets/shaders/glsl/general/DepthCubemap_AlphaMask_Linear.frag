#version 460 core

in Data
{ 
	vec2 text_coord[2];
	vec4 world_frag_pos;
} dataIn;

uniform vec3 center;
uniform float far_plane;

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
		
	 // get distance between fragment and light source
    float distance = length(dataIn.world_frag_pos.xyz - center);
    
    // map to [0;1] range by dividing by far_plane
    distance = distance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = distance;
}  