#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include <assimp/scene.h>        // collects data
#include <assimp/postprocess.h>  // various extra operations

#include "sm.h"

#define ARRAY_LENGTH(a) sizeof(a) / sizeof(a[0])
#define PROJDIR "../"                                                    // path from executable to workspace folder
#define DIR(d) std::string(PROJDIR d)                                    // Get a path from the project directory
#define SPRITEPATH(s) PROJDIR "Sprites/" + s                             // path to sprite given sprite name
#define MODEL_NO_DIR(m) m.substr(0, m.find("."))                         // get model `m` without directory
#define MODELPATH(m) PROJDIR "Models/" + m.substr(0, m.find(".")) + "/"  // get model directory for model `m`

#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b
#define PI 3.14159265358979323846264338327950288f  // Floating-point pi (from GLM)
#define MIN_FLOAT_DIFF 1e-9f

using namespace glm;

namespace Util {
extern vec3 UP;        // World up (0, 1, 0)
extern vec3 DOWN;      // World down (0, -1, 0)
extern vec3 FORWARD;   // World forward (0, 0, -1)
extern vec3 BACKWARD;  // World backward (0, 0, 1)
extern vec3 RIGHT;     // World right (1, 0, 0)
extern vec3 LEFT;      // World left (-1, 0, 0)
extern vec3 X;         // Value of 1 on x axis (1, 0, 0)
extern vec3 Y;         // Value of 1 on y axis (0, 1, 0)
extern vec3 Z;         // Value of 1 on z axis (0, 0, 1)
extern std::random_device rand_dev;
extern std::mt19937 mt_gen;

template <typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

extern std::string readFile(const char* path);
extern float wrap(float val, float min, float max);
extern vec3 wrapV(vec3 val, vec3 min, vec3 max);
extern float sqDist(vec3 a, vec3 b);
extern float clamp(float val, float min, float max);
extern vec3 clampV(vec3 val, vec3 min, vec3 max);
extern float rad(float val);
extern float deg(float val);
extern vec3 rad(vec3 v);
extern vec3 deg(vec3 v);
extern vec3 angleToVec3(float angle);
extern float vec3ToAngle(vec3 v);
extern float lerp(float a, float b, float dt);
extern vec3 lerpV(vec3 a, vec3 b, float dt);
extern vec2 lerpV(vec2 a, vec2 b, float dt);

/* Generate a uniformly-distributed random number in the range (`lo`, `hi`). */
template <arithmetic T>
T random(T lo, T hi) {
    if (std::is_same_v<T, int>) {
        std::uniform_int_distribution<> d(lo, hi);
        return d(mt_gen);
    } else if (std::is_same_v<T, float>) {
        std::uniform_real_distribution<> d(lo, hi);
        return d(mt_gen);
    } else {
        return 0;
    }
}

extern vec2 random2(float lo, float hi);
extern vec3 random3(float lo, float hi);
extern float mapRange(float v, float inLow, float inHigh, float outLow, float outHigh);
extern mat4 lookTowards(vec3 pos, vec3 to);
extern mat4 lookTowards(vec3 pos, vec3 to, vec3 up);
extern vec3 glslColour(vec3 rgb);
extern ivec3 to3D(int index, ivec3 extents);
extern int to1D(ivec3 cell, ivec3 extents);
extern void print(vec2 v);
extern void print(ivec3 v);
extern void print(vec3 v);
extern void print(vec4 v);
extern void print(mat2 m);
extern void print(mat3 m);
extern void print(mat4 m);
extern void print(std::vector<int>);
extern void print(std::vector<float>);
extern aiMatrix4x4 GLMtoAI(const mat4& mat);
extern mat4 aiToGLM(const aiMatrix4x4* from);
extern vec3 aiToGLM(aiVector3D* from);
};  // namespace Util

#endif /* UTIL_H */
