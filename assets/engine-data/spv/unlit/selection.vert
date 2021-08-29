layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;

layout(push_constant) uniform PC {
	mat4 mvp;
};

void main() {
	gl_Position = mvp * vec4(position, 1.f);
}
