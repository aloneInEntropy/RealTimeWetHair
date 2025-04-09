#include "camera.h"
using namespace glm;

void Camera::processView(int xoffset, int yoffset) {
    // Euler angles
    yaw = Util::wrap(yaw + (xoffset * sensitivity), 0.0f, 359.0f);
    pitch = std::clamp(pitch + (yoffset * -sensitivity), -89.5f, 89.5f);
    front =
        normalize(vec3(cos(Util::rad(yaw)) * cos(Util::rad(pitch)),
                       sin(Util::rad(pitch)),
                       sin(Util::rad(yaw)) * cos(Util::rad(pitch))));

    // Update view matrix
    updateViewMatrix();

    // Follow checks
    updateTargets();
}

void Camera::lookAt(vec3 target) {
    target = normalize(target);
    float scaledXOffset = Util::deg(acos(dot(Util::FORWARD, target))) / sensitivity;
    float scaledYOffset = Util::deg(-asin(dot(Util::UP, target))) / sensitivity;
    processView(scaledXOffset, scaledYOffset);
}

void Camera::followTarget(vec3 tPos, vec3 tDir) {
    target = tPos;  // cache
    vec3 fDir = normalize(tDir);
    // rotation from z axis, calculated using rotation about y-axis
    // yaw needs to be negated because as the camera turns right, we want to move leftwards around it, and vice versa
    float angle = atan2f(fDir.z, fDir.x) - Util::rad(yaw);
    float xDist = targetHorizontalDist * sin(angle);
    float zDist = targetHorizontalDist * cos(angle);
    pos = vec3(
        tPos.x + xDist,
        tPos.y - targetVerticalDist,
        tPos.z + zDist);
    followPos = Util::lerpV(followPos, pos, acceleration * SM::delta);
}

void Camera::processMovement() {
    float t_cpos_y = pos.y;  // y-pos of camera before updates
    if (Input::isKeyJustPressed('P')) CAN_FLY = !CAN_FLY;

    if (Input::isKeyPressed('W')) {
        if (CAN_FLY)
            pos += normalize(front) * speed * SM::delta;
        else
            pos += normalize(vec3(front.x, 0, front.z)) * speed * SM::delta;
    }
    if (Input::isKeyPressed('S')) {
        if (CAN_FLY)
            pos -= normalize(front) * speed * SM::delta;
        else
            pos -= normalize(vec3(front.x, 0, front.z)) * speed * SM::delta;
    }
    if (Input::isKeyPressed('A')) {
        if (CAN_FLY)
            pos -= normalize(cross(front, up)) * speed * SM::delta;
        else {
            vec3 c = cross(front, up);
            pos -= normalize(vec3(c.x, 0, c.z)) * speed * SM::delta;
        }
    }
    if (Input::isKeyPressed('D')) {
        if (CAN_FLY)
            pos += normalize(cross(front, up)) * speed * SM::delta;
        else {
            vec3 c = cross(front, up);
            pos += normalize(vec3(c.x, 0, c.z)) * speed * SM::delta;
        }
    }
    if (Input::isKeyPressed(Key::SPACE)) {
        pos += vec3(0, speed * SM::delta, 0);
    };
    if (Input::isKeyPressed(Key::LEFT_SHIFT)) {
        pos -= vec3(0, speed * SM::delta, 0);
    };

    speed = Input::isKeyPressed('E') ? sprintSpeed : baseSpeed;
    if (!CAN_FLY) pos.y = t_cpos_y;  // if can't fly, don't change y_pos

    // Update view matrix
    updateViewMatrix();
}

void Camera::setPosition(vec3 p) {
    pos = p;
    followPos = p;
    processView(0, 0);
}
