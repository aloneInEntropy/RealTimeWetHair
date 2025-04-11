#ifndef MAIN_H
#define MAIN_H

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdio>
#include <iostream>
#include <sstream>

// OpenGL includes
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "camera.h"
#include "common_sim.h"
#include "framebuffer.h"
#include "input.h"
// #include "hair.h"
#include "lighting.h"
#include "shader.h"
#include "staticmesh.h"
#include "grid.h"
#include "simulation.h"
#include "sm.h"
#include "ui.h"
#include "util.h"

/* Pre-defined variables (paths, etc.) */
const char* vert_smesh = PROJDIR "Shaders/staticMesh.vert";
const char* frag_smesh = PROJDIR "Shaders/staticMesh.frag";

/* Meshes */
#define MESH_TEAPOT "teapot.gltf"
#define MESH_SPHERE "sphere.gltf"
#define MESH_MONKEY "monkey.gltf"
#define MESH_HEAD "bighead.gltf"
#define MESH_GUIDE_HEAD "guidehead.gltf"

/* Variables */
Shader *startShader;
Lighting* startLight;
StaticMesh* renderHead, *guideHead;
bool showHead = true;
bool showMonkeys = false;
vec3 lightPos = vec3(0, 3, -5);
vec3 lightCol = vec3(0.2, 1, 1);

Sim::Simulation *sim;
StaticMesh *particle;
vec3 headStartPos = vec3(0, 3, 0);
vec3 headStartRot = vec3(0);

/* Main Functions */
// Initialise function. Runs before the main loop.
void init();
// Display function. Runs inside the main loop. Use for rendering objects to the screen.
void display();
// Update function. Runs inside the main loop. Use for non-rendering tasks such as updating timers
void update();

/* Additional utility functions */
// Resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SM::updateWindow(width, height);
    glViewport(0, 0, width, height);
}

// error/debug message callback, from https://deccer.github.io/OpenGL-Getting-Started/02-debugging/02-debug-callback/
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam) {
    // Ignore certain verbose info messages (particularly ones on Nvidia).
    if (id == 131169 ||
        id == 131185 ||  // NV: Buffer will use video memory
        id == 131218 ||
        id == 131204 ||  // Texture cannot be used for texture mapping
        id == 131222 ||
        id == 131154 ||  // NV: pixel transfer is synchronized with 3D rendering
        id == 0 ||       // gl{Push, Pop}DebugGroup
        id == 2          // Fragment shader performance issue (when using grid, so likely derivative functions)
    )
        return;

    std::stringstream debugMessageStream;
    debugMessageStream << message << '\n';

    std::string src = "";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            src = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            src = "Window Manager";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            src = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            src = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            src = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            src = "Other";
            break;
    }

    std::string typ = "";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typ = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typ = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typ = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typ = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typ = "Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typ = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typ = "Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typ = "Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typ = "Other";
            break;
    }

    std::string sev = "";
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            sev = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            sev = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            sev = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            sev = "Info";
            break;
    }

    fprintf(stderr, "[OpenGL] [%s] [%s] [%s %u] %s\n", src.c_str(), typ.c_str(), sev.c_str(), id, debugMessageStream.str().c_str());
}

#endif /* MAIN_H */
