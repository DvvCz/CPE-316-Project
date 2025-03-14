#pragma once

#include "../lib/board.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"

class App {
public:
    App();
    void run();

private:
    Touchscreen touchscreen;
    Display::Controller lcd;
    Render::LCDRenderer renderer;

    void update();
    void render();
};