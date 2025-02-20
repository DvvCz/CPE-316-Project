#include "ttt.hpp"

std::optional<board_state_t> tileToWinner(tile_state_t tile) {
  switch (tile) {
  case tile_state_t::x:
    return board_state_t::x_won;
  case tile_state_t::o:
    return board_state_t::o_won;
  default:
    return std::nullopt;
  }
}

bool Board::isFull() const {
  for (int row = 0; row < TTT_NUM_ROWS; row++) {
    for (int col = 0; col < TTT_NUM_COLS; col++) {
      if (board[row][col] == tile_state_t::empty) {
        return false;
      }
    }
  }

  return true;
}

Board::Board() {
  for (int row = 0; row < TTT_NUM_ROWS; row++) {
    for (int col = 0; col < TTT_NUM_COLS; col++) {
      board[row][col] = tile_state_t::empty;
    }
  }
}

std::optional<tile_state_t> Board::getTile(uint8_t row, uint8_t col) const {
  if (row < TTT_NUM_ROWS && col < TTT_NUM_COLS) {
    return board[row][col];
  }

  return std::nullopt;
}

bool Board::setTile(uint8_t row, uint8_t col, tile_state_t state) {
  if (row < TTT_NUM_ROWS && col < TTT_NUM_COLS && this->board[row][col] == tile_state_t::empty) {
    this->board[row][col] = state;
    return true;
  }

  return false;
}

board_state_t Board::solveState() const {
  // Check horizontal wins
  for (uint8_t row = 0; row < TTT_NUM_ROWS; row++) {
    tile_state_t currentState = this->board[row][0];
    for (uint8_t col = 1; col < TTT_NUM_COLS; col++) {
      if (this->board[row][col] != currentState) {
        currentState = tile_state_t::empty;
        break;
      }
    }

    if (currentState != tile_state_t::empty) {
      return tileToWinner(currentState).value();
    }
  }

  // Check any vertical wins
  for (uint8_t col = 0; col < TTT_NUM_COLS; col++) {
    tile_state_t currentState = this->board[0][col];
    for (uint8_t row = 1; row < TTT_NUM_ROWS; row++) {
      if (this->board[row][col] != currentState) {
        currentState = tile_state_t::empty;
        break;
      }
    }

    if (currentState != tile_state_t::empty) {
      return tileToWinner(currentState).value();
    }
  }

  // These are hardcoded because I don't want to take the time to make it
  // dynamic when we probably won't ever need to use it

  // Check left diagonal win
  if (this->board[0][0] == this->board[1][1] &&
      this->board[0][0] == this->board[2][2]) {
    return tileToWinner(this->board[0][0]).value();
  }

  // Check right diagonal win
  if (this->board[0][2] == this->board[1][1] &&
      this->board[0][2] == this->board[2][0]) {
    return tileToWinner(this->board[0][2]).value();
  }

  if (this->isFull()) {
    return board_state_t::draw;
  }

  return board_state_t::playing;
}