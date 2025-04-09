#ifndef SM_H
#define SM_H

#include <windows.h>
#include <iostream>
#include "util.h"

class Camera;
class Cubemap;
class Grid;

struct Config {
    bool showSkybox = true;  // Should the skybox be shown?
    bool showGrid = true;    // Should the floor grid be shown?
    bool isCamMode = false;  // Is the user controlling the camera? If `false`, the user will control the cursor.
    glm::vec3 bgColour;
    glm::vec3 gridThickColour;
    glm::vec3 gridThinColour;
};

// Scene Manager singleton
namespace SM {

// Update the window size
extern void updateWindow(int nw, int nh);
// Update the global delta value on each frame.
extern void updateDelta();
// How long the program has been running
extern float getGlobalTime();
// Launch timer lasting `m` seconds
extern void triggerTimer(int m);
// Run timer
extern void runTimer();
// Acknowledge and reset timer
extern void ackTimer();
// Deactivate timer
extern void endTimer();
// Increase the program tick counter
extern void updateTick();
// Switch 3D perspective between first- and third-person camera
extern void switchFirstAndThirdCam();
// Toggle the free camera on or off
extern void toggleFreeCam();
// Initialise the world scene, the grid, and the camera
extern void initScene();
// Draw additional objects on screen, such as the grid or skybox
extern void drawSceneExtras();

// delta time
extern float delta;

// window settings
extern int o_width;        // Original window width
extern int o_height;       // Original window height
extern float o_aspect;     // Original window aspect ratio
extern int width;          // Window width
extern int height;         // Window height
extern int offset_w;       // Window width offset (width/2)
extern int offset_h;       // Window height offset (height/2)
extern float aspect;       // Window aspect ratio
extern float comp_aspect;  // Ratio of the current window size to the original value
extern float w_ratio;
extern float h_ratio;

// start time of the program
extern DWORD startTime;
// Times the `update()` function has been run. Essentially the current frame of the program
extern DWORD tick;

// scene camera
extern Camera *camera;
// world grid
extern Grid *grid;
// Configuration settings
extern Config cfg;
// World skybox
extern Cubemap *skybox;

extern bool isFreeCam;      // in free cam, the player model is loaded and updates but does not move with the camera
extern bool isFirstPerson;  // in first person, the player model is not updated but the camera follows it anyway
extern bool isThirdPerson;  // in third person, the player model is loaded and the camera moves with it

// timer variables
extern float timer_c;
extern float timer_max;
extern bool timer_started;
extern bool timer_finished;

// Mesh variables
extern int unnamedMeshCount;
extern int unnamedStaticMeshCount;
extern int unnamedBoneMeshCount;
extern int unnamedVariantMeshCount;
extern int MAX_NUM_INSTANCES;
};  // namespace SM

#endif /* SM_H */
