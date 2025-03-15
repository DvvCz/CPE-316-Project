#include "ttt.hpp"

#include "../lib/board.hpp"
#include "../lib/display.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"
#include "../lib/uart.hpp"

#include "main.h"

#define DEBOUNCE_TIME 10

static uint32_t lastPressed = 0;
static Touchscreen* globalTouchscreen = nullptr;
static Board* globalBoard = nullptr;

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == TS_IRQ_Pin) {
    auto now = HAL_GetTick();
    if (now - lastPressed < DEBOUNCE_TIME) {
      return;
    }

    lastPressed = now;

    if (globalTouchscreen) {
      globalTouchscreen->irqHook();
    }
  }
}

App::App()
    : board(), touchscreen((SPI_HandleTypeDef*)&hspi3, TouchPins{.clkPort = TS_SCK_GPIO_Port,
                                                                 .clkPin = TS_SCK_Pin,

                                                                 .csPort = TS_CS_GPIO_Port,
                                                                 .csPin = TS_CS_Pin,

                                                                 .dinPort = TS_MOSI_GPIO_Port,
                                                                 .dinPin = TS_MOSI_Pin,

                                                                 .doPort = TS_MISO_GPIO_Port,
                                                                 .doPin = TS_MISO_Pin,

                                                                 .irqPort = TS_IRQ_GPIO_Port,
                                                                 .irqPin = TS_IRQ_Pin}),
      lcd(&hspi2, Display::LCDPins{.mosiPin = LCD_MOSI_Pin,

                                   .sClkPort = LCD_SCK_GPIO_Port,
                                   .sClkPin = LCD_SCK_Pin,

                                   .resetPort = LCD_RESET_GPIO_Port,
                                   .resetPin = LCD_RESET_Pin,

                                   .dcPort = LCD_DC_GPIO_Port,
                                   .dcPin = LCD_DC_Pin}),
      renderer(&lcd) {
}

void App::run() {
  globalTouchscreen = &touchscreen;
  globalBoard = &board;

  touchscreen.init();
  lcd.init();

  while (true) {
    update();
    render();
  }
}

int selectedRow = 0;
int selectedCol = 0;

static int readChar = 0;

TileState currentlyPlaying = TileState::X;
BoardState currentState = BoardState::Playing;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance != USART2)
    return;

  if (readChar == 'a') {
    selectedRow = std::max(selectedRow - 1, 0);
  } else if (readChar == 'd') {
    selectedRow = (selectedRow + 1) % BOARD_NUM_COLS;
  } else if (readChar == 's') {
    selectedCol = std::max(selectedCol - 1, 0);
  } else if (readChar == 'w') {
    selectedCol = (selectedCol + 1) % BOARD_NUM_ROWS;
  } else if (readChar == ' ') {
    if (globalBoard->getTile(selectedRow, selectedCol).value_or(TileState::Empty) != TileState::Empty) {
      return;
    }

    globalBoard->setTile(selectedRow, selectedCol, currentlyPlaying);

    currentlyPlaying = currentlyPlaying == TileState::X ? TileState::O : TileState::X;
    currentState = globalBoard->solveState();

    uartPrintf("Current state: %d\r\n", (int)currentState);
  }
}

bool first = true;

TileState cachedStates[BOARD_NUM_ROWS][BOARD_NUM_COLS] = {{TileState::Empty, TileState::Empty, TileState::Empty},
                                                          {TileState::Empty, TileState::Empty, TileState::Empty},
                                                          {TileState::Empty, TileState::Empty, TileState::Empty}};

bool isSelectedStates[BOARD_NUM_ROWS][BOARD_NUM_COLS] = {
    {false, false, false},
    {false, false, false},
    {false, false, false},
};

void App::reset() {
  first = true;

  for (int row = 0; row < BOARD_NUM_ROWS; row++) {
    for (int col = 0; col < BOARD_NUM_COLS; col++) {
      cachedStates[row][col] = TileState::Empty;
      isSelectedStates[row][col] = false;
    }
  }

  currentlyPlaying = TileState::X;
  currentState = BoardState::Playing;

  globalBoard->clear();
}

void App::update() {
  HAL_UART_Receive_IT(&huart2, (uint8_t*)&readChar, 1);

  if (currentState == BoardState::Playing) {
    return;
  }

  if (currentState == BoardState::XWon) {
    uartPrintf("X won!\r\n");
  } else if (currentState == BoardState::OWon) {
    uartPrintf("O won!\r\n");
  } else if (currentState == BoardState::Draw) {
    uartPrintf("Draw!\r\n");
  }

  uartPrintf("Resetting...\r\n");
  reset();
}

const int BOARD_PADDING = 0;

const Render::LCDColor BOARD_COLOR = Render::BLACK;
const Render::LCDColor GRID_COLOR = Render::WHITE;

const Render::LCDColor X_COLOR = Render::LCDColor::fromRgb8(0, 0, 255);
const Render::LCDColor X_TENTATIVE_COLOR = Render::LCDColor::fromRgb8(50, 50, 255);

const Render::LCDColor O_COLOR = Render::LCDColor::fromRgb8(245, 144, 66);
const Render::LCDColor O_TENTATIVE_COLOR = Render::LCDColor::fromRgb8(245, 174, 120);

const Render::LCDColor BOARD_INVALID_COLOR = Render::LCDColor::fromRgb8(255, 0, 0);

const int TILE_PADDING = 10;

const int TILE_WIDTH = (Display::WIDTH - BOARD_PADDING * 2) / BOARD_NUM_COLS;
const int TILE_INNER_WIDTH = TILE_WIDTH - TILE_PADDING * 2;

const int TILE_HEIGHT = (Display::HEIGHT - BOARD_PADDING * 2) / BOARD_NUM_ROWS;
const int TILE_INNER_HEIGHT = TILE_HEIGHT - TILE_PADDING * 2;

static void drawCross(Render::LCDRenderer renderer, int x, int y, int width, int height) {
  renderer.drawLine(x, y, x + width - 1, y + height - 1);
  renderer.drawLine(x + width - 1, y, x, y + height - 1);
}

void App::renderTile(int row, int col, TileState state, TileState tentativeState) {
  int startX = col * TILE_WIDTH + BOARD_PADDING;
  int startY = row * TILE_HEIGHT + BOARD_PADDING;

  // Clear inner tile
  renderer.setColor(BOARD_COLOR);
  renderer.drawRect(startX + TILE_PADDING, startY + TILE_PADDING, TILE_INNER_WIDTH, TILE_INNER_HEIGHT);

  if (tentativeState != TileState::Empty && state != TileState::Empty) {
    renderer.setColor(BOARD_INVALID_COLOR);
    renderer.drawRectOutline(startX + TILE_PADDING, startY + TILE_PADDING, TILE_INNER_WIDTH, TILE_INNER_HEIGHT);
  }

  switch (state) {
  case TileState::X:
    renderer.setColor(X_COLOR);
    drawCross(renderer, startX + TILE_PADDING, startY + TILE_PADDING, TILE_INNER_WIDTH, TILE_INNER_HEIGHT);
    break;
  case TileState::O:
    renderer.setColor(O_COLOR);
    renderer.drawCircleOutline(startX + TILE_WIDTH / 2, startY + TILE_HEIGHT / 2, 28);
    break;
  case TileState::Empty:
    if (tentativeState == TileState::X) {
      renderer.setColor(X_TENTATIVE_COLOR);
      drawCross(renderer, startX + TILE_PADDING, startY + TILE_PADDING, TILE_INNER_WIDTH, TILE_INNER_HEIGHT);
    } else if (tentativeState == TileState::O) {
      renderer.setColor(O_TENTATIVE_COLOR);
      renderer.drawCircleOutline(startX + TILE_WIDTH / 2, startY + TILE_HEIGHT / 2, 28);
    }
    break;
  }
}

void App::render() {
  bool needsHydration = false;

  if (first) {
    first = false;
    needsHydration = true;

    renderer.setColor(Render::BLACK);
    renderer.drawRect(0, 0, Display::WIDTH, Display::HEIGHT);
  }

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      TileState currentState = board.getTile(row, col).value_or(TileState::Empty);
      bool isSelected = selectedRow == row && selectedCol == col;

      bool wasStateChanged = cachedStates[row][col] != currentState;
      bool wasSelectedChanged = isSelectedStates[row][col] != isSelected;

      // State changed, hydrate tile
      if (wasStateChanged || wasSelectedChanged) {
        needsHydration = true;
        isSelectedStates[row][col] = isSelected;

        cachedStates[row][col] = currentState;
        renderTile(row, col, currentState, isSelected ? currentlyPlaying : TileState::Empty);
      }
    }
  }

  if (!needsHydration) {
    return;
  }

  renderer.setColor(GRID_COLOR);

  // Draw horizontal grid lines
  for (int i = 1; i < BOARD_NUM_COLS; i++) {
    renderer.drawLine(BOARD_PADDING, BOARD_PADDING + i * (Display::HEIGHT / BOARD_NUM_COLS),
                      Display::WIDTH - BOARD_PADDING, BOARD_PADDING + i * Display::HEIGHT / BOARD_NUM_COLS);
  }

  // Draw vertical grid lines
  for (int i = 1; i < BOARD_NUM_ROWS; i++) {
    renderer.drawLine(BOARD_PADDING + i * (Display::WIDTH / BOARD_NUM_ROWS), BOARD_PADDING,
                      BOARD_PADDING + i * Display::WIDTH / BOARD_NUM_ROWS, Display::HEIGHT - BOARD_PADDING);
  }
}