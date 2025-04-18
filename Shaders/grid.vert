#version 460 core

out vec3 fragPos;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 viewPos;
out vec3 eyeCoord;

const vec3 vertices[4] = {
	vec3(-1, 0, -1), // bl
	vec3(1, 0, -1),  // br
	vec3(1, 0, 1),   // tr
	vec3(-1, 0, 1)   // tl
};
const int indices[6] = {0, 2, 1, 2, 0, 3};

void main() {
	int idx = indices[gl_VertexID];
	vec3 vPos = vertices[idx] * 20 * 20;
	vPos.x += viewPos.x;
	vPos.z += viewPos.z;
	vec4 vp = vec4(vPos, 1);
	gl_Position = proj * view * vp;
	fragPos = vPos;
	eyeCoord = vec3(view * vec4(vPos, 1.0));
}
