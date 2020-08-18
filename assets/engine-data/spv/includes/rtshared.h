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

struct hitPayload
{
  vec4 hitValue;
};