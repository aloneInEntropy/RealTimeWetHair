#version 460 core

layout(location = 0) in vec2 pPos;
layout(location = 1) in vec2 pUV;

layout(location = 0) out vec2 uv;

void main() {
	gl_Position = vec4(pPos, 0, 1);
	uv = pUV;
}
