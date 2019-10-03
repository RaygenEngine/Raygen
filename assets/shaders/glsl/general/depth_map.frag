#version 460 core

in vec4 world_pos;

uniform vec3 source_pos;
uniform float far;

void main()
{             
	// get distance between fragment and source
    float distance = length(world_pos.xyz - source_pos);
    
    // map to [0;1] range by dividing by far_plane
    distance = distance / far;
    
    // write this as modified depth
    gl_FragDepth = distance;
}  