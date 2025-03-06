#pragma once

#include "stm32l4xx_hal.h"
#include <optional>
#include <functional>

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

using TouchCallback = std::function<void (int x, int y)>;

#define XFAC      663
#define XOFFSET   (-13)
#define YFAC      894
#define YOFFSET   (-30)

// Touchscreen class
class Touchscreen {
public:
  Touchscreen(SPI_HandleTypeDef* spi, TouchPins pins);

  void init();
  bool isTouched();

  void setCallback(TouchCallback callback);
  void irqHook();

  uint16_t readX();
  uint16_t readY();

private:
  SPI_HandleTypeDef* touchSpi;
  TouchPins touchPins;
  std::optional<TouchCallback> callback = std::nullopt;

  void write(uint8_t data);
  uint8_t read();
  void csLow();
  void csHigh();

  uint8_t spiReadU8();
  uint16_t spiReadU16();
  void spiRead(TouchPins pins, uint8_t* data, uint16_t size);

  void spiWriteU8(uint8_t data);

  void writeU8(uint8_t data);
  uint16_t readADC(uint8_t cmd);
  uint16_t readChannel(uint8_t channel);
};