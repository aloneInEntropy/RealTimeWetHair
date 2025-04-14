#version 460 core

layout (isolines) in;
// layout (location = 0) flat out uint particleID;
patch in vec4 p_1;
patch in vec4 p_2;
layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;

in CS_OUT {
    flat uint particleID;
    float vWeight;
    vec3 tangent;
    vec3 eyeSpacePos;
    vec3 fragPos;
} cs_out[];

out ES_OUT {
    flat uint particleID;
    float vWeight;
    vec3 tangent;
    vec3 eyeSpacePos;
    vec3 fragPos;
} es_out;

void main() {
	// float u = clamp(gl_TessCoord.x, 0, 1);
	float u = gl_TessCoord.x;
	// float v = gl_TessCoord.y;

    vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	// vec4 p2 = gl_in[2].gl_Position;
	// vec4 p3 = gl_in[3].gl_Position;
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
	vec4 p = (b0*p_1 + b1*p0 + b2*p1 + b3*p_2);
    // vec4 p0 = gl_in[0].gl_Position;
	// vec4 p1 = gl_in[1].gl_Position;
	// vec4 p2 = gl_in[2].gl_Position;
	// vec4 p3 = gl_in[3].gl_Position;

    // float b0 = (-1.0*u*u*u + 3.0*u*u - 3.0*u + 1.0) * (1.0/6.0);
    // float b1 = (3.0*u*u*u - 6.0*u*u + 4.0 ) * (1.0/6.0);
    // float b2 = (-3.0*u*u*u + 3.0*u*u + 3.0*u + 1.0) * (1.0/6.0);
    // float b3 = (u*u*u) * (1.0/6.0);
    // vec4 p = b0*p0 + b1*p1 + b2*p2 + b3*p3;
    es_out.fragPos = p.xyz;
    gl_Position = proj * view * p;

    es_out.particleID = cs_out[0].particleID;
    es_out.vWeight = cs_out[0].vWeight;
    es_out.tangent = cs_out[0].tangent; // todo: interpolate
    es_out.eyeSpacePos = cs_out[0].eyeSpacePos;
    es_out.fragPos = cs_out[0].fragPos;
}
