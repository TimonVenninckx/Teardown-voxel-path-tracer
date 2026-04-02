#pragma once

#include <glm/vec2.hpp>

enum class Key
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    TAB,
    CAPS_LOCK,
    SHIFT_LEFT,
    CTRL_LEFT,
    ALT_LEFT,
    ESCAPE,
    RIGHT_SHIFT,
    ENTER,
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    ARROW_LEFT,
    SPACE
};

enum class MouseButton
{
    LEFT,
    MIDDLE,
    RIGHT,
};

class Input
{
public:
    Input(){};
 
    // could make a mouse delta function
    //glm::dvec2 mousePosition;
    virtual void PollEvents() = 0;

    virtual bool IsMousePressed(MouseButton button) = 0;
    virtual bool IsMouseRelease(MouseButton button) = 0;
    virtual bool IsMouseDown(MouseButton button) = 0;

    virtual bool IsKeyPressed(Key key) = 0;
    virtual bool IsKeyRelease(Key key) = 0;
    virtual bool IsKeyDown(Key key) = 0;
    
    // get mouse position relative to window
    // after setting mouse position, mouse delta = (0,0);
    virtual glm::dvec2 GetMousePosition() = 0;
    
    virtual glm::dvec2 GetMouseDelta() = 0;

    virtual int GetScrollWheelDelta() = 0;


};

