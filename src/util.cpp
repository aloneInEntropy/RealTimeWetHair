#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "util.h"

namespace Util {
vec3 UP = vec3(0.f, 1.f, 0.f);
vec3 DOWN = vec3(0.f, -1.f, 0.f);
vec3 FORWARD = vec3(0.f, 0.f, -1.f);
vec3 BACKWARD = vec3(0.f, 0.f, 1.f);
vec3 LEFT = vec3(-1.f, 0.f, 0.f);
vec3 RIGHT = vec3(1.f, 0.f, 0.f);
vec3 X = vec3(1.f, 0.f, 0.f);
vec3 Y = vec3(0.f, 1.f, 0.f);
vec3 Z = vec3(0.f, 0.f, 1.f);
std::random_device rand_dev;
std::mt19937 mt_gen(rand_dev());

std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file " << path << std::endl;
        return "";
    }

    std::string text, line;
    while (getline(file, line)) {
        text += line + "\n";
    }

    file.close();
    text.append("\0");
    return text;
}

// Wrap a value between min and max (inclusive). If val is greater than max, it will wraparound to min and begin climbing from there, and vice versa.
float wrap(float val, float min, float max) {
    return fmod(min + (val - min), max - min);
}

vec3 wrapV(vec3 val, vec3 min, vec3 max) {
    return vec3(
        wrap(val.x, min.x, max.x),
        wrap(val.y, min.y, max.y),
        wrap(val.z, min.z, max.z));
}

// Clamp a value between a minimum and maximum so that val cannot be greater than max or smaller than min.
float clamp(float val, float min, float max) {
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}

vec3 clampV(vec3 val, vec3 min, vec3 max) {
    return vec3(
        clamp(val.x, min.x, max.x),
        clamp(val.y, min.y, max.y),
        clamp(val.z, min.z, max.z));
}

// Linearly interpolate between `a` and `b` at time factor `t`. `t` should be mapped to 0-1
float lerp(float a, float b, float t) {
    if (t > 1) t = 1;
    if (t < 0) t = 0;
    return (a * (1 - t)) + b * t;
}

// Lerp the individual components of a vector
vec3 lerpV(vec3 a, vec3 b, float dt) {
    return vec3(
        lerp(a.x, b.x, dt),
        lerp(a.y, b.y, dt),
        lerp(a.z, b.z, dt));
}
// Lerp the individual components of a vector
vec2 lerpV(vec2 a, vec2 b, float dt) {
    return vec2(
        lerp(a.x, b.x, dt),
        lerp(a.y, b.y, dt));
}

// Generate a vector with uniformly-distributed random components in the cubed range `(vec2(lo)`, `vec2(hi))`.
vec2 random2(float lo, float hi) {
    std::uniform_real_distribution<> d(lo, hi);
    return vec2(
        d(mt_gen),
        d(mt_gen));
}

// Generate a vector with uniformly-distributed random components in the cubed range `(vec3(lo)`, `vec3(hi))`.
vec3 random3(float lo, float hi) {
    std::uniform_real_distribution<> d(lo, hi);
    return vec3(
        d(mt_gen),
        d(mt_gen),
        d(mt_gen));
}

// Map a value `v` from input range `[inLow, inHigh]` to output range `[outLow, outHigh]`
// https://stackoverflow.com/a/5732390
float mapRange(float v, float inLow, float inHigh, float outLow, float outHigh) {
    float slope = (outHigh - outLow) / (inHigh - inLow);
    return outLow + slope * (v - inLow);
}

// Compute the squared distance between `a` and `b`. Use `glm::distance` for actual distance
float sqDist(vec3 a, vec3 b) {
    vec3 diff = a - b;
    return dot(diff, diff);
}

// Convert degrees to radians
float rad(float val) { return glm::radians(val); }

// Convert radians to degrees
float deg(float val) { return glm::degrees(val); }

// Convert degrees to radians
vec3 rad(vec3 v) { return vec3(rad(v.x), rad(v.y), rad(v.z)); }

// Convert radians to degrees
vec3 deg(vec3 v) { return vec3(deg(v.x), deg(v.y), deg(v.z)); }

// Convert an angle in degrees to a vec3. The angle will use the y-axis as the floor; i.e., the y-axis value will be 0.
vec3 angleToVec3(float angle) {
    return vec3(-cosf(rad(angle)), 0, -sinf(rad(angle)));
}

// Convert a vector to an angle in degrees. The angle will use the y-axis as the floor; i.e., the y-axis value will be ignored during the calculation.
float vec3ToAngle(vec3 v) {
    return deg(atan2(v.y, v.x));
}

// Create a matrix at position `from` looking in the direction `to`. Uses the global up direction (0, 1, 0)
mat4 lookTowards(vec3 from, vec3 to) {
    mat4 mat = translate(mat4(1), from);
    to = normalize(to);
    return mat * inverse(lookAt(from, from + to, UP));
}

// Create a matrix at position `from` looking in the direction `at`.
mat4 lookTowards(vec3 from, vec3 to, vec3 up) {
    mat4 mat = translate(mat4(1), from);
    to = normalize(to);
    return mat * inverse(lookAt(from, from + to, up));
}

// Convert an RGB value in the colour space (0-255) to a GLSL colour in the colour space (0-1)
vec3 glslColour(vec3 rgb) {
    return rgb / 255.f;
}

// Convert a 1D index (e.g., in a list) to a 3D point given the extents (i.e., max sizes) of the 3D grid and the size of each (cubic) cell
ivec3 to3D(int index, ivec3 extents) {
    return {
        index % (extents.x),
        (index / extents.x) % extents.y,
        index / (extents.y * extents.z)};
}

// Convert a 1D index (e.g., in a list) to a 3D point given the width, height, and depth of the 3D grid
// `ID = x * y(maxX) + z(maxX * maxY) == x * maxX * (y + z * maxY)`
int to1D(ivec3 cell, ivec3 extents) {
    return cell.x + extents.x * (cell.y + cell.z * extents.y);
}

void print(vec4 v) {
    printf("(%f, %f, %f, %f)\n", v.x, v.y, v.z, v.w);
}

void print(vec3 v) {
    printf("(%f, %f, %f)\n", v.x, v.y, v.z);
}

void print(ivec3 v) {
    printf("(%d, %d, %d)\n", v.x, v.y, v.z);
}

void print(vec2 v) {
    printf("(%f, %f)\n", v.x, v.y);
}

void print(mat2 m) {
    printf("\n");
    printf("[%.2f][%.2f]\n", m[0][0], m[1][0]);
    printf("[%.2f][%.2f]\n", m[0][1], m[1][1]);
}

void print(mat3 m) {
    printf("\n");
    printf("[%.2f][%.2f][%.2f]\n", m[0][0], m[1][0], m[2][0]);
    printf("[%.2f][%.2f][%.2f]\n", m[0][1], m[1][1], m[2][1]);
    printf("[%.2f][%.2f][%.2f]\n", m[0][2], m[1][2], m[2][2]);
}

void print(mat4 m) {
    printf("\n");
    printf("[%.2f][%.2f][%.2f][%.2f]\n", m[0][0], m[1][0], m[2][0], m[3][0]);
    printf("[%.2f][%.2f][%.2f][%.2f]\n", m[0][1], m[1][1], m[2][1], m[3][1]);
    printf("[%.2f][%.2f][%.2f][%.2f]\n", m[0][2], m[1][2], m[2][2], m[3][2]);
    printf("[%.2f][%.2f][%.2f][%.2f]\n", m[0][3], m[1][3], m[2][3], m[3][3]);
}

void print(std::vector<int> vs) {
    printf("[");
    for (int i = 0; i < vs.size(); ++i) {
        printf("%d", vs[i]);
        if (i != vs.size() - 1)
            printf(", ");
    }
    printf("]\n");
}

void print(std::vector<float> vs) {
    printf("[");
    for (int i = 0; i < vs.size(); ++i) {
        printf("%.6f", vs[i]);
        if (i != vs.size() - 1)
            printf(", ");
    }
    printf("]\n");
}

aiMatrix4x4 GLMtoAI(const mat4& mat) {
    aiMatrix4x4 p = aiMatrix4x4();
    p.a1 = mat[0][0];
    p.a2 = mat[0][1];
    p.a3 = mat[0][2];
    p.a4 = mat[0][3];
    p.b1 = mat[1][0];
    p.b2 = mat[1][1];
    p.b3 = mat[1][2];
    p.b4 = mat[1][3];
    p.c1 = mat[2][0];
    p.c2 = mat[2][1];
    p.c3 = mat[2][2];
    p.c4 = mat[2][3];
    p.d1 = mat[3][0];
    p.d2 = mat[3][1];
    p.d3 = mat[3][2];
    p.d4 = mat[3][3];
    return p;
}

mat4 aiToGLM(const aiMatrix4x4* from) {
    mat4 to;
    to[0][0] = (GLfloat)from->a1;
    to[0][1] = (GLfloat)from->b1;
    to[0][2] = (GLfloat)from->c1;
    to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2;
    to[1][1] = (GLfloat)from->b2;
    to[1][2] = (GLfloat)from->c2;
    to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3;
    to[2][1] = (GLfloat)from->b3;
    to[2][2] = (GLfloat)from->c3;
    to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4;
    to[3][1] = (GLfloat)from->b4;
    to[3][2] = (GLfloat)from->c4;
    to[3][3] = (GLfloat)from->d4;
    return to;
}

vec3 aiToGLM(aiVector3D* from) {
    return vec3(
        from->x,
        from->y,
        from->z);
}

}  // namespace Util