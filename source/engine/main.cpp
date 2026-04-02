#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include <iostream>
#include "Engine.h"
#include "WindowsInput.h"
#include "WindowsWindow.h"
#include "Debug.h"

#include "../Application.h"
#include "Timer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

int main()
{
    WindowsWindow window;
    WindowsInput input(window.window);
    Debug debug;

    Engine::Init(&window, &input, &debug);
    Application app;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.window, true);

    ImGui_ImplOpenGL3_Init("#version 130");

    Timer t;
    while (window.IsActive())
    {
        if (input.IsKeyPressed(Key::ESCAPE))
            break;
        float time = t.elapsed();
        t.reset();
        app.Run(time);

        window.SwapBuffer();
        input.PollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
