#include "ttt.hpp"

#include "../lib/ttt.hpp"
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
{
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

}

void App::renderTile(int row, int col, TileState state) {
  int margin = 10;

  int width = (Display::WIDTH - margin) / 3;
  int height = (Display::HEIGHT - margin) / 3;

  int startX = (Display::WIDTH - margin) / 3 * col + margin;
  int startY = (Display::HEIGHT - margin) / 3 * row + margin;

  // First clear it
  renderer.setColor(Render::BLACK);
  renderer.drawRect(startX, startY, width, height);

  renderer.setColor(Render::BLUE);

  switch (state) {
    case TileState::X:
      renderer.drawLine(startX, startY, startX + width, startY + height);
      renderer.drawLine(startX, startY + height, startX + width, startY);
      break;
    case TileState::O:
      renderer.drawCircleOutline(startX + width / 2, startY + height / 2, width / 2);
      break;
    case TileState::Empty:
      break;
  }

}

void App::render() {
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      renderTile(row, col, board.getTile(row, col).value_or(TileState::Empty));
    }
  }

  renderer.drawGrid();
}