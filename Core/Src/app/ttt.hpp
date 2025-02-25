#pragma once



#include "../lib/render.hpp"

extern Display::Controller lcd;
extern Render::LCDRenderer render;


namespace App {
    void init();  // App initialization function
} // namespace App

// Enum to represent the state of a tile (Empty, X, O)
enum class TileState { Empty, X, O };

// Board class handles the Tic-Tac-Toe logic
class Board {
public:
    Board();  // Constructor - Initializes an empty board

    // Set a tile (X or O) at a specific position
    void setTile(int row, int col, TileState state);

    // Get the current state of a tile (X, O, or Empty)
    TileState getTile(int row, int col) const;

    // Check if there's a winner (X or O)
    TileState checkWinner(); //returns the winner, if there is no winner, returns the Empty tilestate

    // Check if the board is full (used to detect draw)
    bool isFull();

    // Reset the board to an empty state
    void reset();

    void writeTile(int row, int col, TileState state); //this is for writing the tile on the grid on the lcd

private:
    // 3x3 grid to store the board state
    TileState grid[3][3];
};
