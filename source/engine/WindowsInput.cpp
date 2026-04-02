#include "WindowsInput.h"
#include <iostream>
#include <GLFW/glfw3.h>

WindowsInput* input;

void scroll_callback(GLFWwindow*, double, double yoffset)
{
    input->queuedScrollDelta += static_cast<int>(yoffset);
}

WindowsInput::WindowsInput(GLFWwindow* window)
    : window{ window }
{
    input = this;
    glfwSetScrollCallback(window, scroll_callback);
}

bool WindowsInput::IsKeyPressed(Key key)
{
    return cur.keyDown[(int)key] == true && prev.keyDown[(int)key] == false;
}

bool WindowsInput::IsKeyRelease(Key key)
{
    return cur.keyDown[(int)key] == false && prev.keyDown[(int)key] == true;
}

bool WindowsInput::IsKeyDown(Key key)
{
    return cur.keyDown[(int)key];
}

bool WindowsInput::IsMousePressed(MouseButton button)
{
    return cur.mouseDown[(int)button] == true && prev.mouseDown[(int)button] == false;
}

bool WindowsInput::IsMouseRelease(MouseButton button)
{
    return cur.mouseDown[(int)button] == false && prev.mouseDown[(int)button] == true;
}

bool WindowsInput::IsMouseDown(MouseButton button)
{
    return cur.mouseDown[(int)button];
}



void WindowsInput::PollEvents() {

    prev.mousePosition = cur.mousePosition;

    glfwGetCursorPos(window, &cur.mousePosition.x, &cur.mousePosition.y);

    std::memcpy(prev.keyDown, cur.keyDown, sizeof(cur.keyDown));
    std::memcpy(prev.mouseDown, cur.mouseDown, sizeof(cur.mouseDown));


    scrollDelta = queuedScrollDelta;
    queuedScrollDelta = 0;

    // very lazily get all the events
    cur.keyDown[(int)Key::A]            = glfwGetKey(window, GLFW_KEY_A);
    cur.keyDown[(int)Key::B]            = glfwGetKey(window, GLFW_KEY_B);
    cur.keyDown[(int)Key::C]            = glfwGetKey(window, GLFW_KEY_C);
    cur.keyDown[(int)Key::D]            = glfwGetKey(window, GLFW_KEY_D);
    cur.keyDown[(int)Key::E]            = glfwGetKey(window, GLFW_KEY_E);
    cur.keyDown[(int)Key::F]            = glfwGetKey(window, GLFW_KEY_F);
    cur.keyDown[(int)Key::G]            = glfwGetKey(window, GLFW_KEY_G);
    cur.keyDown[(int)Key::H]            = glfwGetKey(window, GLFW_KEY_H);
    cur.keyDown[(int)Key::I]            = glfwGetKey(window, GLFW_KEY_I);
    cur.keyDown[(int)Key::J]            = glfwGetKey(window, GLFW_KEY_J);
    cur.keyDown[(int)Key::K]            = glfwGetKey(window, GLFW_KEY_K);
    cur.keyDown[(int)Key::L]            = glfwGetKey(window, GLFW_KEY_L);
    cur.keyDown[(int)Key::M]            = glfwGetKey(window, GLFW_KEY_M);
    cur.keyDown[(int)Key::N]            = glfwGetKey(window, GLFW_KEY_N);
    cur.keyDown[(int)Key::O]            = glfwGetKey(window, GLFW_KEY_O);
    cur.keyDown[(int)Key::P]            = glfwGetKey(window, GLFW_KEY_P);
    cur.keyDown[(int)Key::Q]            = glfwGetKey(window, GLFW_KEY_Q);
    cur.keyDown[(int)Key::R]            = glfwGetKey(window, GLFW_KEY_R);
    cur.keyDown[(int)Key::S]            = glfwGetKey(window, GLFW_KEY_S);
    cur.keyDown[(int)Key::T]            = glfwGetKey(window, GLFW_KEY_T);
    cur.keyDown[(int)Key::U]            = glfwGetKey(window, GLFW_KEY_U);
    cur.keyDown[(int)Key::V]            = glfwGetKey(window, GLFW_KEY_V);
    cur.keyDown[(int)Key::W]            = glfwGetKey(window, GLFW_KEY_W);
    cur.keyDown[(int)Key::X]            = glfwGetKey(window, GLFW_KEY_X);
    cur.keyDown[(int)Key::Y]            = glfwGetKey(window, GLFW_KEY_Y);
    cur.keyDown[(int)Key::Z]            = glfwGetKey(window, GLFW_KEY_Z);
    cur.keyDown[(int)Key::NUM_0]        = glfwGetKey(window, GLFW_KEY_0);
    cur.keyDown[(int)Key::NUM_1]        = glfwGetKey(window, GLFW_KEY_1);
    cur.keyDown[(int)Key::NUM_2]        = glfwGetKey(window, GLFW_KEY_2);
    cur.keyDown[(int)Key::NUM_3]        = glfwGetKey(window, GLFW_KEY_3);
    cur.keyDown[(int)Key::NUM_4]        = glfwGetKey(window, GLFW_KEY_4);
    cur.keyDown[(int)Key::NUM_5]        = glfwGetKey(window, GLFW_KEY_5);
    cur.keyDown[(int)Key::NUM_6]        = glfwGetKey(window, GLFW_KEY_6);
    cur.keyDown[(int)Key::NUM_7]        = glfwGetKey(window, GLFW_KEY_7);
    cur.keyDown[(int)Key::NUM_8]        = glfwGetKey(window, GLFW_KEY_8);
    cur.keyDown[(int)Key::NUM_9]        = glfwGetKey(window, GLFW_KEY_9);
    cur.keyDown[(int)Key::TAB]          = glfwGetKey(window, GLFW_KEY_TAB);
    cur.keyDown[(int)Key::CAPS_LOCK]    = glfwGetKey(window, GLFW_KEY_CAPS_LOCK);
    cur.keyDown[(int)Key::SHIFT_LEFT]   = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    cur.keyDown[(int)Key::CTRL_LEFT]    = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    cur.keyDown[(int)Key::ALT_LEFT]     = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
    cur.keyDown[(int)Key::ESCAPE]       = glfwGetKey(window, GLFW_KEY_ESCAPE);
    cur.keyDown[(int)Key::RIGHT_SHIFT]  = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
    cur.keyDown[(int)Key::ENTER]        = glfwGetKey(window, GLFW_KEY_ENTER);
    cur.keyDown[(int)Key::ARROW_UP]     = glfwGetKey(window, GLFW_KEY_UP);
    cur.keyDown[(int)Key::ARROW_RIGHT]  = glfwGetKey(window, GLFW_KEY_RIGHT);
    cur.keyDown[(int)Key::ARROW_DOWN]   = glfwGetKey(window, GLFW_KEY_DOWN);
    cur.keyDown[(int)Key::ARROW_LEFT]   = glfwGetKey(window, GLFW_KEY_LEFT);
    cur.keyDown[(int)Key::SPACE]        = glfwGetKey(window, GLFW_KEY_SPACE);



    cur.mouseDown[(int)MouseButton::LEFT]   = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    cur.mouseDown[(int)MouseButton::RIGHT]  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    cur.mouseDown[(int)MouseButton::MIDDLE] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

}

glm::dvec2 WindowsInput::GetMousePosition()
{
    return cur.mousePosition;
}

glm::dvec2 WindowsInput::GetMouseDelta()
{
    return cur.mousePosition - prev.mousePosition;
}

int WindowsInput::GetScrollWheelDelta()
{
    return scrollDelta;
}
