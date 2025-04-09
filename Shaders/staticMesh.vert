#version 460 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_texture;
layout(location = 3) in vec3 vertex_tangent;
layout(location = 4) in mat4 instance_trans;

out vec3 FragPos;
out vec2 uv;
out mat3 TBN;
out vec3 oN;
out vec3 eyeCoord;

uniform mat4 view;
uniform mat4 proj;

void main() {
	FragPos = vec3(instance_trans * vec4(vertex_position, 1.0));
	uv = vertex_texture;
	mat3 NMat = mat3(transpose(inverse(instance_trans)));
	vec3 T = normalize(NMat * vertex_tangent);
	vec3 N = normalize(NMat * vertex_normal);
	oN = N;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = transpose(mat3(T, B, N));
	gl_Position = proj * view * vec4(FragPos, 1.0);
	eyeCoord = vec3(view * instance_trans * vec4(vertex_position, 1.0));
}
