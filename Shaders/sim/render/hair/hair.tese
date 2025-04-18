#version 460 core

layout (isolines, equal_spacing, cw) in;

patch in vec4 p_1;
patch in vec4 p_2;
layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;

in CS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 eyeSpacePos;
    vec3 fragPos;
} cs_out[];

out ES_OUT {
    flat uint particleID;
    float vWeight;
    vec3 eyeSpacePos;
    vec3 fragPos;
} es_out;

void main() {
	float u = clamp(gl_TessCoord.x, 0, 1);

    vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	float b0 = (0.f) - (1.f * u) + (2.f * u * u) - (1.f * u * u * u);
	float b1 = (2.f) + (0.f * u) - (5.f * u * u) + (3.f * u * u * u);
	float b2 = (0.f) + (1.f * u) + (4.f * u * u) - (3.f * u * u * u);
	float b3 = (0.f) + (0.f * u) - (1.f * u * u) + (1.f * u * u * u);
	vec4 p = (b0*p_1 + b1*p0 + b2*p1 + b3*p_2);

    es_out.fragPos = p.xyz;
    gl_Position = proj * view * p;

    es_out.particleID = cs_out[0].particleID;
    es_out.vWeight = mix(cs_out[0].vWeight, cs_out[1].vWeight, u);
    es_out.eyeSpacePos = mix(cs_out[0].eyeSpacePos, cs_out[1].eyeSpacePos, u);
    es_out.fragPos = mix(cs_out[0].fragPos, cs_out[1].fragPos, u);
}
