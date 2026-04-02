#pragma once
#include "BaseWindow.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class WindowsWindow : public BaseWindow
{
public:
    WindowsWindow();
    ~WindowsWindow();

    void SwapBuffer()override;
    void SetWindowName(const std::string& name)override;
    void SetMousePosition(int x, int y)override;
    void SetCursorMode(CursorMode mode)override;
    void GetWindowSize(int& width, int& height)override;

    bool IsActive()override;

    GLFWwindow* window;
private:
    void PollEvents()override;
};

