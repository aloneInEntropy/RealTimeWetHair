#ifndef CAMERA_H
#define CAMERA_H

#include "util.h"
#include "sm.h"
#include "input.h"

class Camera {
   private:
    float acceleration = 10;  // lerp acceleration factor; to be multiplied by delta
   public:
    // Create a new Camera object with its default screen settings
    Camera() {
        orthographicProjection = ortho<float>(0.f, SM::width, SM::height, 0.f, -1.f, 1.f);
        perspectiveProjection = perspective(Util::rad(FOV), aspectRatio, nearClipDist, farClipDist);
        updateViewMatrix();
    }

    // Create a new Camera object with a given near clip distance, far clip distance, and aspect ratio
    Camera(float _nearClipDist, float _farClipDist, float aspect) : Camera() {
        nearClipDist = _nearClipDist;
        farClipDist = _farClipDist;
        aspectRatio = aspect;
    }

    // Process and update the camera's view matrix.
    // `xoffset` and `yoffset` are the mouse position offsets used to dictate the direction the player is looking.
    void processView(int xoffset, int yoffset);

    // Process camera movement through the world space.
    void processMovement();

    // Follow the target position `pos` who is pointing in the direction `dir` at some fixed distance
    // If `dir == vec3(0, 0, -1)`, the camera will act like an arcball camera, orbiting `pos`
    void followTarget(vec3 pos, vec3 dir);

    // Orbit the target `pos` at a variable fixed distance
    // The camera will act like an arcball camera, orbiting `pos`
    void orbitTarget(vec3 pos) { followTarget(pos, Util::FORWARD); }

    // Set the camera position to some location
    void setPosition(vec3 p);

    // Get the world view matrix for the camera.
    mat4 getViewMatrix() { return view; }

    // Look at some target. The camera's Euler angles are adjusted to match the direction of the target
    void lookAt(vec3 target);

    // Update the camera's view matrix
    void updateViewMatrix() {
        right = normalize(cross(front, Util::UP));
        up = normalize(cross(right, front));
        view = SM::isFreeCam ? glm::lookAt(pos, pos + front, up) : glm::lookAt(followPos, followPos + front, up);
    }

    void updateTargets() {
        if (SM::isFirstPerson) {
            targetVerticalDist = -0.25;
            targetHorizontalDist = fps_zm;
        } else {
            targetVerticalDist = targetDist * sin(Util::rad(pitch));
            targetHorizontalDist = targetDist * cos(Util::rad(pitch));
        }
    }

    // Get the orthographic projection matrix for the camera.
    mat4 getOrthographicMatrix() { return orthographicProjection; }

    // Get the perspective projection matrix for the camera.
    mat4 getPerspectiveMatrix() { return perspectiveProjection; }

    // Variables
    vec3 pos = vec3(0.0f, 1.0f, 0.0f);  // Camera position
    vec3 followPos = vec3(0.0f);        // Camera follow position; used for interpolated movement
    vec3 front = Util::FORWARD;         // Camera direction
    vec3 up = Util::UP;                 // Camera up direction
    vec3 right = Util::RIGHT;           // Camera right direction
    vec3 target;                        // Camera target position
    mat4 view;                          // Model view matrix
    mat4 orthographicProjection;        // Model orthographic projection matrix
    mat4 perspectiveProjection;         // Model perspective projection matrix
    float aspectRatio = 16.f / 9.f;     // Aspect ratio of camera

    float targetDist = 12;       // Camera distance to target (fixed)
    float fps_zm = -3;           // Camera zoom in first person mode
    float targetHorizontalDist;  // Camera horizontal distance to target
    float targetVerticalDist;    // Camera vertical distance to target

    float nearClipDist = 0.1f;   // Distance to the near clip plane
    float farClipDist = 1000.f;  // Distance to the far clip plane
    float base_FOV = 60.0f;      // Normal (minimum) field of view
    float max_FOV = 105.0f;      // Maximum field of view
    float FOV = base_FOV;        // Camera field of view
    float pitch = 0.0;           // x-axis rotation (vertical)
    float yaw = 270.0;           // y-axis rotation (horizontal)
    float roll = 0.0;            // z-axis rotation
    float sensitivity = .15;     // Camera sensitivity
    float baseSpeed = 60.0f;      // Walking speed (movement speed)
    float sprintSpeed = 125.0f;   // Sprinting speed (movement speed)
    float speed = baseSpeed;     // Camera speed (movement speed)
    bool CAN_FLY = true;         // can the player fly (i.e., move on y-axis at will)?
};

#endif /* CAMERA_H */
