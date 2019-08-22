#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "CommonStructs.h"


rtDeclareVariable( float3, shading_normal,   attribute shading_normal, );
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable( float3, texcoord,         attribute texcoord, );
rtDeclareVariable( float2, barycentrics,     attribute barycentrics, );

rtBuffer<Core::Vertex> vertex_buffer;
rtBuffer<uint3>   index_buffer;

RT_PROGRAM void triangle_attributes()
{
    const uint3  v_idx = index_buffer[rtGetPrimitiveIndex()];
    const float3 v0    = vertex_buffer[v_idx.x].position;
    const float3 v1    = vertex_buffer[v_idx.y].position;
    const float3 v2    = vertex_buffer[v_idx.z].position;
    const float3 Ng    = optix::cross( v1 - v0, v2 - v0 );

    geometric_normal = optix::normalize( Ng );

    barycentrics = rtGetTriangleBarycentrics();
	// could be used if missing uvs
    //texcoord = make_float3( barycentrics.x, barycentrics.y, 0.0f );

    shading_normal = vertex_buffer[v_idx.y].normal * barycentrics.x + vertex_buffer[v_idx.z].normal * barycentrics.y
        + vertex_buffer[v_idx.x].normal * ( 1.0f-barycentrics.x-barycentrics.y );



    const float2 t0 = vertex_buffer[v_idx.x].uv;
    const float2 t1 = vertex_buffer[v_idx.y].uv;
    const float2 t2 = vertex_buffer[v_idx.z].uv;
    texcoord = make_float3( t1*barycentrics.x + t2*barycentrics.y + t0*(1.0f-barycentrics.x-barycentrics.y) );
    
}

