#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 vps[6];

in Data
{ 
	vec2 text_coord[2];
} dataIn[];

out Data
{ 
	vec2 text_coord[2];
	vec4 world_frag_pos;
} dataOut;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
			dataOut.world_frag_pos = gl_in[i].gl_Position;
            gl_Position = vps[face] * dataOut.world_frag_pos;
			dataOut.text_coord[0] = dataIn[i].text_coord[0];
			dataOut.text_coord[1] = dataIn[i].text_coord[1];
            EmitVertex();
        }    
        EndPrimitive();
    }
}