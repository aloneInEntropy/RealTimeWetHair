#include "input.h"

namespace Input {
void initialiseInputMap() {
    for (int i = 0; i < KEY_COUNT; ++i) {
        keys[i] = KeyState::RELEASED;
    }
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        buttons[i] = ClickState::RELEASED;
    }
}
bool isKeyPressed(int key) {
    return keys[key] == KeyState::PRESSED || keys[key] == KeyState::HELD;
}
bool isKeyPressed(Key key) {
    return keys[(int)key] == KeyState::PRESSED || keys[(int)key] == KeyState::HELD;
}
bool isKeyReleased(int key) {
    return keys[key] == KeyState::RELEASED;
}
bool isKeyReleased(Key key) {
    return keys[(int)key] == KeyState::RELEASED;
}
bool isKeyJustPressed(int key) {
    return keys[key] == KeyState::PRESSED && old_keys[key] == KeyState::RELEASED;
}
bool isKeyJustPressed(Key key) {
    return keys[(int)key] == KeyState::PRESSED && old_keys[(int)key] == KeyState::RELEASED;
}
bool isKeyJustReleased(int key) {
    return keys[key] == KeyState::RELEASED && (old_keys[key] == KeyState::PRESSED || old_keys[key] == KeyState::HELD);
}
bool isKeyJustReleased(Key key) {
    return keys[(int)key] == KeyState::RELEASED && (old_keys[(int)key] == KeyState::PRESSED || old_keys[(int)key] == KeyState::HELD);
}

bool isButtonPressed(int button) {
    return buttons[button] == ClickState::PRESSED;
}
bool isButtonPressed(MouseButton button) {
    return buttons[(int)button] == ClickState::PRESSED;
}
bool isButtonReleased(int button) {
    return buttons[button] == ClickState::RELEASED;
}
bool isButtonReleased(MouseButton button) {
    return buttons[(int)button] == ClickState::RELEASED;
}
bool isButtonJustPressed(int button) {
    return buttons[button] == ClickState::PRESSED && old_buttons[button] == ClickState::RELEASED;
}
bool isButtonJustPressed(MouseButton button) {
    return buttons[(int)button] == ClickState::PRESSED && old_buttons[(int)button] == ClickState::RELEASED;
}
bool isButtonJustReleased(int button) {
    return buttons[button] == ClickState::RELEASED && old_buttons[button] == ClickState::PRESSED;
}
bool isButtonJustReleased(MouseButton button) {
    return buttons[(int)button] == ClickState::RELEASED && old_buttons[(int)button] == ClickState::PRESSED;
}
void updateKey(int key, int state) {
    keys[key] = (KeyState)state;
}
void updateOldKeys() {
    memcpy(old_keys, keys, KEY_COUNT * sizeof(int));
}
void updateOldButtons() {
    memcpy(old_buttons, buttons, BUTTON_COUNT * sizeof(int));
}

void updateMousePosition(int x, int y) {
    mouseDX = x - mouse.x;
    mouseDY = y - mouse.y;
    mouse = glm::vec2(x, y);
}
void updateMouseClickState(int button, int state) {
    buttons[button] = (ClickState)state;
    mousePressed = isButtonPressed(MouseButton::LEFT);
}

glm::vec2 mouse;
glm::vec2 lastMousePressed;
int mouseDX = 0;
int mouseDY = 0;
bool mousePressed = false;
bool windowFocused = true;

KeyState* old_keys = new KeyState[KEY_COUNT];
KeyState* keys = new KeyState[KEY_COUNT];
ClickState* old_buttons = new ClickState[BUTTON_COUNT];
ClickState* buttons = new ClickState[BUTTON_COUNT];

}  // namespace Input
