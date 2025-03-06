#include "board.hpp"

std::optional<BoardState> tileToWinner(TileState tile) {
  switch (tile) {
  case TileState::X:
    return BoardState::XWon;
  case TileState::O:
    return BoardState::OWon;
  default:
    return std::nullopt;
  }
}

bool Board::isFull() const {
  for (int row = 0; row < BOARD_NUM_ROWS; row++) {
    for (int col = 0; col < BOARD_NUM_COLS; col++) {
      if (board[row][col] == TileState::Empty) {
        return false;
      }
    }
  }

  return true;
}

Board::Board() {
  for (int row = 0; row < BOARD_NUM_ROWS; row++) {
    for (int col = 0; col < BOARD_NUM_COLS; col++) {
      board[row][col] = TileState::Empty;
    }
  }
}

std::optional<TileState> Board::getTile(uint8_t row, uint8_t col) const {
  if (row < BOARD_NUM_ROWS && col < BOARD_NUM_COLS) {
    return board[row][col];
  }

  return std::nullopt;
}

bool Board::setTile(uint8_t row, uint8_t col, TileState state) {
  if (row < BOARD_NUM_ROWS && col < BOARD_NUM_COLS && this->board[row][col] == TileState::Empty) {
    this->board[row][col] = state;
    return true;
  }

  return false;
}

BoardState Board::solveState() const {
  // Check horizontal wins
  for (uint8_t row = 0; row < BOARD_NUM_ROWS; row++) {
    TileState currentState = this->board[row][0];
    for (uint8_t col = 1; col < BOARD_NUM_COLS; col++) {
      if (this->board[row][col] != currentState) {
        currentState = TileState::Empty;
        break;
      }
    }

    if (currentState != TileState::Empty) {
      return tileToWinner(currentState).value();
    }
  }

  // Check any vertical wins
  for (uint8_t col = 0; col < BOARD_NUM_COLS; col++) {
    TileState currentState = this->board[0][col];
    for (uint8_t row = 1; row < BOARD_NUM_ROWS; row++) {
      if (this->board[row][col] != currentState) {
        currentState = TileState::Empty;
        break;
      }
    }

    if (currentState != TileState::Empty) {
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
    return BoardState::Draw;
  }

  return BoardState::Playing;
}