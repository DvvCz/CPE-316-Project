#include "calibrate.hpp"

#include "../lib/board.hpp"
#include "../lib/display.hpp"
#include "../lib/render.hpp"
#include "../lib/touchscreen.hpp"
#include "../lib/uart.hpp"

#include "main.h"

#define DEBOUNCE_TIME 10

static uint32_t lastPressed = 0;
static Touchscreen* globalTouchscreen = nullptr;

// extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//   if (GPIO_Pin == TS_IRQ_Pin) {
//     auto now = HAL_GetTick();
//     if (now - lastPressed < DEBOUNCE_TIME) {
//       return;
//     }

//     lastPressed = now;

//     if (globalTouchscreen) {
//       globalTouchscreen->irqHook();
//     }
//   }
// }

CalibrateApp::CalibrateApp()
    : touchscreen((SPI_HandleTypeDef*)&hspi3, TouchPins{.clkPort = TS_SCK_GPIO_Port,
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

struct CalibrationData {
  int xMin;
  int xMax;
  int yMin;
  int yMax;
};

void CalibrateApp::run() {
  globalTouchscreen = &touchscreen;

  touchscreen.init();

  lcd.init();

  CalibrationData data = {};

  // Bottom left corner
  renderer.setColor(Render::RED);
  renderer.drawRect(0, 0, 20, 20);

  bool noData = true;

  touchscreen.setCallback([&](int x, int y) {
    data.xMin = x;
    data.yMin = y;
    noData = false;
  });

  while (noData) {
  }

  renderer.setColor(Render::GREEN);
  renderer.drawRect(0, 0, 20, 20);

  HAL_Delay(1000);

  noData = true;

  // Top right corner
  renderer.setColor(Render::RED);
  renderer.drawRect(Display::WIDTH - 20, Display::HEIGHT - 20, 20, 20);

  touchscreen.setCallback([&](int x, int y) {
    data.xMax = x;
    data.yMax = y;
    noData = false;
  });

  while (noData) {
  }

  renderer.setColor(Render::GREEN);
  renderer.drawRect(Display::WIDTH - 20, Display::HEIGHT - 20, 20, 20);

  HAL_Delay(1000);

  noData = true;

  // Top left corner
  renderer.setColor(Render::RED);
  renderer.drawRect(0, Display::HEIGHT - 20, 20, 20);

  touchscreen.setCallback([&](int x, int y) {
    data.xMin = (data.xMin + x) / 2;
    data.yMax = (data.yMax + y) / 2;
    noData = false;
  });

  while (noData) {
  }

  renderer.setColor(Render::GREEN);
  renderer.drawRect(0, Display::HEIGHT - 20, 20, 20);

  HAL_Delay(1000);

  noData = true;

  // Bottom right corner
  renderer.setColor(Render::RED);
  renderer.drawRect(Display::WIDTH - 20, 0, 20, 20);

  touchscreen.setCallback([&](int x, int y) {
    data.xMax = (data.xMax + x) / 2;
    data.yMin = (data.yMin + y) / 2;
    noData = false;
  });

  while (noData) {
  }

  renderer.setColor(Render::GREEN);
  renderer.drawRect(Display::WIDTH - 20, 0, 20, 20);

  HAL_Delay(1000);

  uartPrintf("Calibration data: %d, %d, %d, %d\r\n", data.xMin, data.xMax, data.yMin, data.yMax);

  touchscreen.setCallback([=](int x, int y) {
    float u = (float)(x - data.xMin) / (data.xMax - data.xMin);
    float v = (float)(y - data.yMin) / (data.yMax - data.yMin);

    int resolvedX = u * Display::WIDTH;
    int resolvedY = v * Display::HEIGHT;

    renderer.setColor(Render::BLUE);
    renderer.drawCircleOutline(resolvedX, resolvedY, 2);
  });

  while (true) {
    HAL_Delay(1000);
  }
}