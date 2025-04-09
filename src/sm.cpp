#include "sm.h"
#include "input.h"
#include "camera.h"   // fwd
#include "grid.h"     // fwd
#include "cubemap.h"  // fwd

namespace SM {

enum CAMERA_MODE {
    FIRST = 0,
    THIRD = 1,
    FREE = 2
};
inline float delta = 0.0f;
DWORD startTime = 0;
DWORD tick = 0;

float timer_c = 0;
float timer_max = 0;
bool timer_started = false;
bool timer_finished = false;

int o_width = 2560;
int o_height = 1494;
float o_aspect = (o_width * 1.f) / o_height;
int width = o_width;
int height = o_height;
float aspect = width / height;
int offset_w = width / 2;
int offset_h = height / 2;
float comp_aspect = aspect / o_aspect;
float w_ratio = 1;
float h_ratio = 1;

bool isFreeCam = true;
bool isFirstPerson = false;
bool isThirdPerson = false;
CAMERA_MODE camMode = CAMERA_MODE::FREE;
CAMERA_MODE lastCamMode = CAMERA_MODE::FREE;

int unnamedMeshCount = 0;
int unnamedStaticMeshCount = 0;
int unnamedBoneMeshCount = 0;
int unnamedVariantMeshCount = 0;
int MAX_NUM_INSTANCES = 10000;

Camera *camera = nullptr;
Grid *grid = nullptr;
Config cfg = Config();
Cubemap *skybox;

void initScene() {
    camera = new Camera(0.1f, 1000.0f, (float)SM::width / (float)SM::height);
    skybox = new Cubemap({
        DIR("Models/sb/right.png"),
        DIR("Models/sb/left.png"),
        DIR("Models/sb/top.png"),
        DIR("Models/sb/bottom.png"),
        DIR("Models/sb/front.png"),
        DIR("Models/sb/back.png"),
    });
    grid = new Grid();
    cfg.bgColour = Util::glslColour({35, 20, 30});
    cfg.gridThickColour = Util::glslColour({127, 127, 127});
    cfg.gridThinColour = Util::glslColour({192, 192, 192});
}

void drawSceneExtras() {
    if (cfg.showSkybox) SM::skybox->render();
    if (cfg.showGrid) grid->render();
}

void updateDelta() {
    static DWORD last_time = 0;
    DWORD curr_time = timeGetTime();
    if (last_time == 0) last_time = curr_time;
    delta = (curr_time - last_time) * 0.001f;
    last_time = curr_time;
}

void updateWindow(int nw, int nh) {
    width = nw;
    height = nh;
    offset_w = nw / 2;
    offset_h = nh / 2;
    aspect = (nw * 1.f) / nh;
    w_ratio = (width * 1.f) / o_width;
    h_ratio = (height * 1.f) / o_height;
    comp_aspect = aspect / o_aspect;
}

void switchFirstAndThirdCam() {
    if (isFreeCam) return;
    camMode = (CAMERA_MODE)Util::wrap(camMode + 1, CAMERA_MODE::FIRST, CAMERA_MODE::THIRD + 1);
    isFirstPerson = camMode == CAMERA_MODE::FIRST;
    isThirdPerson = camMode == CAMERA_MODE::THIRD;
    printf("SWITCHED CAMERA MODE: %s\n", isFirstPerson ? "first person" : (isThirdPerson ? "third person" : "free camera"));
}

void toggleFreeCam() {
    if (camMode == CAMERA_MODE::FREE) {
        // disable free cam
        camMode = lastCamMode;
        isFreeCam = false;
        isFirstPerson = camMode == CAMERA_MODE::FIRST;
        isThirdPerson = camMode == CAMERA_MODE::THIRD;
        printf("SWITCHED CAMERA MODE: %s\n", isFirstPerson ? "first person" : (isThirdPerson ? "third person" : "free camera"));
    } else {
        // enable free cam
        lastCamMode = camMode;
        camMode = CAMERA_MODE::FREE;
        isFreeCam = true;
        isFirstPerson = false;
        isThirdPerson = false;
        printf("SWITCHED CAMERA MODE: free camera\n");
    }
}

float getGlobalTime() {
    return ((float)(timeGetTime() - SM::startTime)) * 0.001f;
}

void triggerTimer(int m) {
    if (!timer_started) {
        timer_started = true;
        timer_c = m;
        timer_max = m;
    } else {
        printf("TIMER ALREADY ACTIVE; CALL ackTimer() TO RESET\n");
    }
}

void runTimer() {
    if (timer_started) {
        timer_c -= delta;
        if (timer_c <= 0) {
            timer_finished = true;
            return;
        }
    }
}

void ackTimer() {
    timer_started = false;
    timer_finished = false;
}

void endTimer() {
    float timer_c = 0;
    float timer_max = 0;
    bool timer_started = false;
    bool timer_finished = false;
}

void updateTick() {
    tick++;
}

}  // namespace SM
