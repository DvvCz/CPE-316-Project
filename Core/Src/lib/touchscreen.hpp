#pragma once

/*
  Touchscreen library for the MSP2807

  Baud Rate: 5MBits/s
  Data Size: 16 Bits
  Frame Format: Motorola
  Big Endian (MSB First)
*/

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

enum CSMode { Active = 0, Idle = 1 };

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

  uint16_t currentX;
  uint16_t currentY;

  void update();

  uint8_t spiReadU8();
  uint16_t spiReadU16();

  void spiWriteU8(uint8_t data);
  void spiWriteU16(uint16_t data);

  uint16_t spiTransferU16(uint16_t data);

  void setCSMode(const CSMode mode);
};