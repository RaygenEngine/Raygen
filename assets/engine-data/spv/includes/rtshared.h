struct Vertex
{
  float posX;
  float posY;
  float posZ;

  float nrmX;
  float nrmY;
  float nrmZ;

  float tngX;
  float tngY;
  float tngZ;

  float u;
  float v;
};


struct OldVertex
{
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec2 uv;
};

// CHECK: alignment
struct hitPayload
{
// this is a union of Li and Lo
  vec3 radiance;
  
  int depth;

  //vec3 debug;
  //bool debugDone;

  uint seed;
};