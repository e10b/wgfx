#pragma once

#include <unordered_map>
//#include <SDL.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

enum class Key {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down,
    Menu,
    Place,
    Break,
    Run,
    Fly,

    Unknown,

    // Add more keys as needed
};

class Input
{
public:
    // Singleton instance
    static Input& Instance()
    {
        static Input instance;
        return instance;
    }


    Input();

    bool getKeyPressed(Key key);
    bool getKey(Key key);
    bool getKeyReleased(Key key);
    glm::vec2 getDeltaMouse();
    float getScroll();
    void update();



private:
    enum class KeyState {
        None,
        Pressed,
        Released,
        Held
    };

    std::unordered_map<Key, KeyState> keyStates;

    // Helper function to convert SDL_Scancode to Key
    Key SDLScancodeToKey(SDL_Scancode scancode) const;

    // Get the current state of a key
    KeyState getKeyState(Key key) const;

    // Prevent copying and assignment
    Input(Input const&) = delete;
    void operator=(Input const&) = delete;
};