#pragma once

#include "../lib/board.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"

class App {
public:
    App();
    void run();

private:
    Board board;
    Touchscreen touchscreen;
    Display::Controller lcd;
    Render::LCDRenderer renderer;

    void reset();

    void update();
    void render();

    void renderTile(int row, int col, TileState state, TileState tentativeState = TileState::Empty);
};