#pragma once
#include <string>

enum class CursorMode {
    Hidden,
    Show,
    HideCaptured,
    ShowCaptured,
};


class BaseWindow
{
public:
    // polls event updates input call only once per frame
    virtual void PollEvents() = 0;
    // swap the graphics buffer
    virtual void SwapBuffer() = 0;
    virtual void SetWindowName(const std::string& name) = 0;

    // after setting mouse position, mouse delta = (0,0);
    virtual void SetMousePosition(int x, int y) = 0;

    virtual void SetCursorMode(CursorMode mode) = 0;

    virtual void GetWindowSize(int& width, int& height) = 0;
    virtual bool IsActive() = 0;
};

