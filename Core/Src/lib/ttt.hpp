#pragma once

#include <optional>
#include <cstdint>

#define TTT_NUM_ROWS 3
#define TTT_NUM_COLS 3

enum TileState {
    Empty,
    X,
    O
};

enum BoardState {
    Playing,
    XWon,
    OWon,
    Draw
};

class Board {
private:
    TileState board[3][3];

    bool isFull() const;
public:
    Board();

    std::optional<TileState> getTile(uint8_t row, uint8_t col) const;
    bool setTile(uint8_t row, uint8_t col, TileState state);
    BoardState solveState() const;
};