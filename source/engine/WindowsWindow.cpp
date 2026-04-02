#include "BaseWindow.h"
#include "WindowsWindow.h"
#include <iostream>

void error_callback(int /*error*/, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}



WindowsWindow::WindowsWindow()
{
    glfwSetErrorCallback(error_callback);
    // glfw

    if (!glfwInit()) {
        throw "failed to intialize glfw";
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1280, 720, "VoxelEngine-Timon", NULL, NULL);
    if (!window) {
        std::cout << "Failed to initialize window\n";
        glfwTerminate();
        throw "failed to intialize window";
    }
    //glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    gladLoadGL();
    
    /*glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);*/

}

WindowsWindow::~WindowsWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowsWindow::PollEvents()
{
    glfwPollEvents();
}

void WindowsWindow::SwapBuffer()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapBuffers(window);

    PollEvents();
}


void WindowsWindow::SetWindowName(const std::string& name)
{
    glfwSetWindowTitle(window, name.c_str());
}

void WindowsWindow::SetMousePosition(int x, int y) {

    //std::cout << "SETTING MOUSE POSITION TO x:" << x << " y:" << y << std::endl;
    glfwSetCursorPos(window, (double)x, (double)y);
}

void WindowsWindow::SetCursorMode(CursorMode mode)
{
    /*
    https://www.glfw.org/docs/latest/group__input.html#gaa92336e173da9c8834558b54ee80563b

    GLFW_CURSOR_NORMAL makes the cursor visible and behaving normally.
    GLFW_CURSOR_HIDDEN makes the cursor invisible when it is over the content area of the window but does not restrict the cursor from leaving.
    GLFW_CURSOR_DISABLED hides and grabs the cursor, providing virtual and unlimited cursor movement. This is useful for implementing for example 3D camera controls.
    GLFW_CURSOR_CAPTURED makes the cursor visible and confines it to the content area of the window.
    */

    switch (mode) {
    case CursorMode::Hidden:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case CursorMode::Show:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case CursorMode::ShowCaptured:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        break;
    case CursorMode::HideCaptured:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    }

}


void WindowsWindow::GetWindowSize(int& width, int& height)
{
    glfwGetWindowSize(window, &width, &height);
}

bool WindowsWindow::IsActive()
{
    return !glfwWindowShouldClose(window);
}
