#include "ttt.hpp"

#include "../lib/board.hpp"
#include "../lib/display.hpp"
#include "../lib/uart.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"

#include "main.h"

#define DEBOUNCE_TIME 50

uint32_t lastPressed = 0;
Touchscreen* globalTouchscreen = nullptr;

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

App::App() :
  board(),
  touchscreen(&hspi3, TouchPins {
    .clkPort = TS_SCK_GPIO_Port,
    .clkPin = TS_SCK_Pin,

    .csPort = TS_NSS_GPIO_Port,
    .csPin = TS_NSS_Pin,

    .dinPort = TS_MISO_GPIO_Port,
    .dinPin = TS_MISO_Pin,

    .doPort = TS_MOSI_GPIO_Port,
    .doPin = TS_MOSI_Pin,

    .irqPort = TS_IRQ_GPIO_Port,
    .irqPin = TS_IRQ_Pin
  }),
  lcd(&hspi2, Display::LCDPins {
    .mosiPin = LCD_MOSI_Pin,

    .sClkPort = LCD_SCK_GPIO_Port,
    .sClkPin = LCD_SCK_Pin,

    .resetPort = LCD_RESET_GPIO_Port,
    .resetPin = LCD_RESET_Pin,

    .dcPort = LCD_DC_GPIO_Port,
    .dcPin = LCD_DC_Pin
  }),
  renderer(&lcd)
{ }

void App::run() {
  globalTouchscreen = &touchscreen;

  touchscreen.init();
  touchscreen.setCallback([](int x, int y) {
    uartPrintf("Touch at %d, %d\r\n", x, y);
  });

  lcd.init();

  while (true) {
    update();
    render();
  }
}

void App::update() {
  board.setTile(0, 0, TileState::X);
  board.setTile(1, 1, TileState::O);
}

const int BOARD_PADDING = 0;

const int TILE_PADDING = 10;

const int TILE_WIDTH = (Display::WIDTH - BOARD_PADDING * 2) / BOARD_NUM_COLS;
const int TILE_INNER_WIDTH = TILE_WIDTH - TILE_PADDING * 2;

const int TILE_HEIGHT = (Display::HEIGHT - BOARD_PADDING * 2) / BOARD_NUM_ROWS;
const int TILE_INNER_HEIGHT = TILE_HEIGHT - TILE_PADDING * 2;

void App::renderTile(int row, int col, TileState state) {
  int startX = col * TILE_WIDTH + BOARD_PADDING;
  int startY = row * TILE_HEIGHT + BOARD_PADDING;

  renderer.setColor(Render::BLUE);

  switch (state) {
    case TileState::X:
      // Bottom left to top right
      renderer.drawLine(startX + TILE_PADDING, startY + TILE_PADDING, startX + TILE_WIDTH - TILE_PADDING, startY + TILE_HEIGHT - TILE_PADDING);

      // Top left to bottom right
      renderer.drawLine(startX + TILE_PADDING, startY + TILE_HEIGHT - TILE_PADDING, startX + TILE_WIDTH - TILE_PADDING, startY + TILE_PADDING);
      break;
    case TileState::O:
      renderer.drawCircleOutline(startX + TILE_WIDTH / 2, startY + TILE_HEIGHT / 2, TILE_WIDTH / 2 - TILE_PADDING);
      break;
    case TileState::Empty:
      break;
  }
}

void App::render() {
  // Backdrop
  renderer.setColor(Render::BLACK);
  renderer.drawRect(0, 0, Display::WIDTH, Display::HEIGHT);

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      renderTile(row, col, board.getTile(row, col).value_or(TileState::Empty));
    }
  }

  renderer.setColor(Render::WHITE);

  // Draw horizontal grid lines
  for (int i = 1; i < BOARD_NUM_COLS; i++) {
    renderer.drawLine(BOARD_PADDING, BOARD_PADDING + i * (Display::HEIGHT / BOARD_NUM_COLS), Display::WIDTH - BOARD_PADDING, BOARD_PADDING + i * Display::HEIGHT / BOARD_NUM_COLS);
  }

  // Draw vertical grid lines
  for (int i = 1; i < BOARD_NUM_ROWS; i++) {
    renderer.drawLine(BOARD_PADDING + i * (Display::WIDTH / BOARD_NUM_ROWS), BOARD_PADDING, BOARD_PADDING + i * Display::WIDTH / BOARD_NUM_ROWS, Display::HEIGHT - BOARD_PADDING);
  }

  HAL_Delay(100000);
}