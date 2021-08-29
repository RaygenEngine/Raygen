layout(location=0) out vec2 uv;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;
layout(location = 4) in ivec4 joint;
layout(location = 5) in vec4 weight;

layout(push_constant) uniform PC {
	mat4 mvp;
} push;


layout(std430, set = 2, binding = 0) readonly buffer SSBO_Joints {
	mat4 invBindMatrix[];
} jm;

void main() {
	mat4 skinMat = 
		weight.x * jm.invBindMatrix[joint.x] +
		weight.y * jm.invBindMatrix[joint.y] +
		weight.z * jm.invBindMatrix[joint.z] +
		weight.w * jm.invBindMatrix[joint.w];

	gl_Position = push.mvp * skinMat * vec4(position, 1.0);

	uv = textCoord;
}
