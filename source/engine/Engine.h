#pragma once

#include "BaseWindow.h"
#include "Input.h"
#include "Debug.h"


class Engine {
public:
    static void Init(BaseWindow* window, Input* input, Debug* debug) {
        Instance().window = window;
        Instance().input = input;
        Instance().debug = debug;
    }

    static Engine& Instance() { return instance; }

    BaseWindow* window{};
    Input* input{};
    Debug* debug{};

private:
    Engine() {};
    static Engine instance;
    
};

