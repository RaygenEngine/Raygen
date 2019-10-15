#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 vps[6];

in Data
{ 
	vec2 textCoord[2];
} dataIn[];

out Data
{ 
	vec2 textCoord[2];
	vec4 wcs_fragPos;
} dataOut;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
			dataOut.wcs_fragPos = gl_in[i].gl_Position;
            gl_Position = vps[face] * dataOut.wcs_fragPos;
			dataOut.textCoord[0] = dataIn[i].textCoord[0];
			dataOut.textCoord[1] = dataIn[i].textCoord[1];
            EmitVertex();
        }    
        EndPrimitive();
    }
}