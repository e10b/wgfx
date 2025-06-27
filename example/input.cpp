#include "input.h"

Input::Input()
{
    // Initialize all key states
    for (int i = 0; i <= static_cast<int>(Key::Unknown); ++i) {
        keyStates[static_cast<Key>(i)] = KeyState::None;
    }
}

bool Input::getKeyPressed(Key key)
{
    return getKeyState(key) == KeyState::Pressed;
}

bool Input::getKey(Key key)
{
    return getKeyState(key) == KeyState::Held;
}

bool Input::getKeyReleased(Key key)
{
    return getKeyState(key) == KeyState::Released;
}

glm::vec2 Input::getDeltaMouse()
{
    float x, y;
    SDL_GetRelativeMouseState(&x, &y);
    return glm::vec2(x, y);
}

float Input::getScroll()
{
    return 0.0f; // Replace with actual implementation if needed
}

void Input::update()
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_KEY_DOWN: {
            if (event.key.repeat == 0) {
                Key key = SDLScancodeToKey(event.key.scancode);
                if (key != Key::Unknown) {
                    keyStates[key] = KeyState::Pressed;
                }
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            Key key = SDLScancodeToKey(event.key.scancode);
            if (key != Key::Unknown) {
                keyStates[key] = KeyState::Released;
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL:
            // handle scroll events
            break;
        }
    }

    // Update held keys
    for (auto& pair : keyStates) {
        if (pair.second == KeyState::Pressed) {
            pair.second = KeyState::Held;
        }
        else if (pair.second == KeyState::Released) {
            pair.second = KeyState::None;
        }
    }
}

Input::KeyState Input::getKeyState(Key key) const
{
    auto it = keyStates.find(key);
    if (it != keyStates.end()) {
        return it->second;
    }
    return KeyState::None;
}

Key Input::SDLScancodeToKey(SDL_Scancode scancode) const
{
    switch (scancode) {
    case SDL_SCANCODE_W: return Key::Forward;
    case SDL_SCANCODE_S: return Key::Backward;
    case SDL_SCANCODE_A: return Key::Left;
    case SDL_SCANCODE_D: return Key::Right;
    case SDL_SCANCODE_SPACE: return Key::Up;
    case SDL_SCANCODE_LCTRL: return Key::Down;
    case SDL_SCANCODE_ESCAPE: return Key::Menu;
    // case SDL_BUTTON_LEFT: return Key::Place;
    // case SDL_BUTTON_RIGHT: return Key::Break;
    case SDL_SCANCODE_LSHIFT: return Key::Run;
    case SDL_SCANCODE_F1: return Key::Fly;

    default: return Key::Unknown;
    }
}