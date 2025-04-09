#ifndef INPUT_H
#define INPUT_H
#include <windows.h>
#include <iostream>
#include <unordered_map>

#include "util.h"

// number of keys in the key map
#define KEY_COUNT 348
// number of keys in the button map
#define BUTTON_COUNT 8

// Enumerator of (useful) keys
enum class Key {
    SPACE = ' ',
    ESCAPE = 256,
    ENTER = 257,
    TAB = 258,
    BACKSPACE = 259,
    INSERT = 260,
    DEL = 261,
    RIGHT = 262,
    LEFT = 263,
    DOWN = 264,
    UP = 265,
    LEFT_SHIFT = 340,
    LEFT_CONTROL = 341,
    LEFT_ALT = 342,
    RIGHT_SHIFT = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT = 346,
};

// The possible states each key can be in
enum class KeyState {
    RELEASED,  // GLFW_RELEASE
    PRESSED,   // GLFW_PRESS
    HELD,      // GLFW_REPEAT
};

enum class MouseButton {
    LEFT,      // GLFW_MOUSE_BUTTON_LEFT == GLFW_MOUSE_BUTTON_1
    RIGHT,     // GLFW_MOUSE_BUTTON_RIGHT == GLFW_MOUSE_BUTTON_2
    MIDDLE,    // GLFW_MOUSE_BUTTON_MIDDLE == GLFW_MOUSE_BUTTON_3
    BUTTON_4,  // GLFW_MOUSE_BUTTON_4
    BUTTON_5,  // GLFW_MOUSE_BUTTON_5
    BUTTON_6,  // GLFW_MOUSE_BUTTON_6
    BUTTON_7,  // GLFW_MOUSE_BUTTON_7
    BUTTON_8,  // GLFW_MOUSE_BUTTON_8
};

enum class ClickState {
    RELEASED,  // GLFW_RELEASE
    PRESSED    // GLFW_PRESS
};

namespace Input {
// set of keys and their pressed state on the previous frame
extern KeyState* old_keys;
// set of keys and their pressed state on the current frame
extern KeyState* keys;
// set of buttons and their pressed state on the previous frame
extern ClickState* old_buttons;
// set of buttons and their pressed state on the current frame
extern ClickState* buttons;

// is `key` being pressed?
extern bool isKeyPressed(int key);
// is `key` being pressed?
extern bool isKeyPressed(Key key);

// has `key` been released?
extern bool isKeyReleased(int key);
// has `key` been released?
extern bool isKeyReleased(Key key);

// is this the first frame `key` was pressed?
extern bool isKeyJustPressed(Key key);
// is this the first frame `key` was pressed?
extern bool isKeyJustPressed(int key);

// is this the first frame `key` was released?
extern bool isKeyJustReleased(int key);
// is this the first frame `key` was released?
extern bool isKeyJustReleased(Key key);

// is `button` being pressed?
extern bool isButtonPressed(int button);
// is `button` being pressed?
extern bool isButtonPressed(MouseButton button);
// has `button` been released?
extern bool isButtonReleased(int button);
// has `button` been released?
extern bool isButtonReleased(MouseButton button);
// is this the first frame `button` was pressed?
extern bool isButtonJustPressed(int button);
// is this the first frame `button` was pressed?
extern bool isButtonJustPressed(MouseButton button);
// is this the first frame `button` was released?
extern bool isButtonJustReleased(int button);
// is this the first frame `button` was released?
extern bool isButtonJustReleased(MouseButton button);

// update the state of `key`
extern void updateKey(int key, int state);
// initialise all keys to unpressed
extern void initialiseInputMap();
// update old keys to new keys. required for `isKeyJust*` functions
extern void updateOldKeys();
// update old buttons to new buttons. required for `isMouseButtonJust*` functions
extern void updateOldButtons();
// update mouse position
extern void updateMousePosition(int x, int y);
// update mouse click state
extern void updateMouseClickState(int button, int state);

// current mouse position
extern glm::vec2 mouse;
// position of mouse when it was (left-)clicked
extern glm::vec2 lastMousePressed;
// how much the x-position of the mouse changed on the last frame
extern int mouseDX;
// how much the y-position of the mouse changed on the last frame
extern int mouseDY;
// is the mouse being (left-)clicked?
extern bool mousePressed;
// is the window in focus?
extern bool windowFocused;
}  // namespace Input

#endif /* INPUT_H */
