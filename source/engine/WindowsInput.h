#pragma once
#include "Input.h"
#include <glm/vec2.hpp>


struct InputState
{
    bool keyDown[256]{};
    bool mouseDown[3]{};

    glm::dvec2 mousePosition{};
};

struct GLFWwindow;

class WindowsInput : public Input
{
public:
    WindowsInput(GLFWwindow* window);

    void PollEvents()override;

    GLFWwindow* window{};

    InputState prev;
    InputState cur;
    
    int scrollDelta{};
    int queuedScrollDelta{};
    
    bool IsMousePressed(MouseButton button)override;
    bool IsMouseRelease(MouseButton button)override;
    bool IsMouseDown(MouseButton button)override;
    
    bool IsKeyPressed(Key key)override;
    bool IsKeyRelease(Key key)override;
    bool IsKeyDown(Key key)override;

    glm::dvec2 GetMousePosition()override;
    glm::dvec2 GetMouseDelta()override;

    int GetScrollWheelDelta()override;

};

