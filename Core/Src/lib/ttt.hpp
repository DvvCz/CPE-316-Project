#ifndef TTT_H
#define TTT_H

#include <optional>
#include <cstdint>

#define TTT_NUM_ROWS 3
#define TTT_NUM_COLS 3

typedef enum {
    empty,
    x,
    o
} tile_state_t;

typedef enum {
    playing,
    x_won,
    o_won,
    draw
} board_state_t;

class Board {
private:
    tile_state_t board[3][3];

    bool isFull() const;
public:
    Board();

    std::optional<tile_state_t> getTile(uint8_t row, uint8_t col) const;
    bool setTile(uint8_t row, uint8_t col, tile_state_t state);
    board_state_t solveState() const;
};

#endif