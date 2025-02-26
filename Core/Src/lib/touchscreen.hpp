#pragma once

#include "stm32l4xx_hal.h"
#include <optional>

// Pin definitions
typedef struct {
  GPIO_TypeDef* clkPort;
  uint16_t clkPin;

  GPIO_TypeDef* csPort;
  uint16_t csPin;

  GPIO_TypeDef* dinPort;
  uint16_t dinPin;

  GPIO_TypeDef* doPort;
  uint16_t doPin;

  GPIO_TypeDef* irqPort;
  uint16_t irqPin;
} TouchPins;

typedef void (*TouchCallback)(int x, int y);

// Touchscreen class
class Touchscreen {
public:
  Touchscreen(SPI_HandleTypeDef* spi, TouchPins pins);

  void init();
  bool isTouched();
  void readTouch(uint16_t& x, uint16_t& y);

  void setCallback(TouchCallback callback);
  void irqHook();

private:
  SPI_HandleTypeDef* touchSpi;
  TouchPins touchPins;
  std::optional<TouchCallback> callback;

  void write(uint8_t data);
  uint8_t read();
  void csLow();
  void csHigh();

  uint8_t spiReadU8();
  void spiRead(TouchPins pins, uint8_t* data, uint16_t size);
  void spiWriteU8(uint8_t data);

  uint16_t readTouchX();
  uint16_t readTouchY();

  static const uint8_t TS_READ_X = 0x90;  // command for reading X position   THESE TWO MAYBE DIFFERENT DEPENDING ON TS CONTROLLER
  static const uint8_t TS_READ_Y = 0xD0;  //  command for reading Y position
};