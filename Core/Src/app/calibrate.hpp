#pragma once

#include "../lib/board.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"

class CalibrateApp {
public:
  CalibrateApp();
  void run();

private:
  Touchscreen touchscreen;
  Display::Controller lcd;
  Render::LCDRenderer renderer;

  void update();
  void render();
};