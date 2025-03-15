
#include "touchscreen.hpp"

#include "stm32l4xx_hal.h"

#include "uart.hpp"

// #define MSP2807_CMD_READ_X 0xD8
// #define MSP2807_CMD_READ_Y 0xD9

// #define MSP2807_CMD_READ_X 0xD0
// #define MSP2807_CMD_READ_Y 0x90

#define MSP2807_CMD_READ_X 0x91
#define MSP2807_CMD_READ_Y 0xD1

#define DELAY 10000

// Touchscreen class definition
Touchscreen::Touchscreen(SPI_HandleTypeDef* spi, TouchPins pins) {
  touchSpi = spi;
  touchPins = pins;
  currentX = 0;
  currentY = 0;
}

// Initializes touchscreen (setup SPI)
void Touchscreen::init() {
  setCSMode(Idle);
}

void Touchscreen::setCallback(TouchCallback callback) {
  this->callback = callback;
}

// Function to check if touchscreen is touched (IRQ pin)
bool Touchscreen::isTouched() {
  return HAL_GPIO_ReadPin(touchPins.irqPort, touchPins.irqPin) == GPIO_PIN_RESET;
}

void Touchscreen::irqHook() {
  if (callback.has_value()) {
    int x = readX();
    int y = readY();
    (callback.value())(x, y);
  }
}

static uint16_t bestTwoAvg(uint16_t x, uint16_t y, uint16_t z) {
  uint16_t dA = x > y ? x - y : y - x;
  uint16_t dB = x > z ? x - z : z - x;
  uint16_t dC = z > y ? z - y : y - z;

  if (dA <= dB && dA <= dC) {
    return (x + y) >> 1;
  } else if (dB <= dA && dB <= dC) {
    return (x + z) >> 1;
  } else {
    return (y + z) >> 1;
  }
}

void Touchscreen::update() {
  setCSMode(Active);

  uint16_t results[5] = {};

  spiWriteU16(0x91); // ignore first result. just to turn on

  results[0] = spiTransferU16(0xD1) >> 3; // y
  results[1] = spiTransferU16(0x91) >> 3; // x
  results[2] = spiTransferU16(0xD1) >> 3; // y
  results[3] = spiTransferU16(0x91) >> 3; // x

  results[4] = spiTransferU16(0xD0) >> 3; // last y
  results[5] = spiTransferU16(0) >> 3;    // shutoff?

  setCSMode(Idle);

  uint16_t x = bestTwoAvg(results[0], results[1], results[2]);
  uint16_t y = bestTwoAvg(results[1], results[3], results[5]);

  if (x > 4095) {
    x = 4095;
  }

  if (y > 4095) {
    y = 4095;
  }

  currentX = x;
  currentY = y;
}

// Configured for big endian transmission.

uint8_t Touchscreen::spiReadU8() {
  uint8_t data = 0;
  HAL_SPI_Receive(touchSpi, &data, 1, DELAY);
  return data;
}

uint16_t betole(uint16_t data) {
  return (data >> 8) | (data << 8); // swap bytes
}

uint16_t letobe(uint16_t data) {
  return (data >> 8) | (data << 8); // swap bytes
}

uint16_t Touchscreen::spiReadU16() {
  uint16_t data = 0;
  HAL_SPI_Receive(touchSpi, (uint8_t*)&data, 2, DELAY);
  return data;
}

void Touchscreen::spiWriteU8(uint8_t byte) {
  HAL_SPI_Transmit(touchSpi, &byte, 1, DELAY);
}

void Touchscreen::spiWriteU16(uint16_t word) {
  // uint16_t data = letobe(word);
  HAL_SPI_Transmit(touchSpi, (uint8_t*)&word, 2, DELAY);
}

void Touchscreen::setCSMode(const CSMode mode) {
  HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, mode == Active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

uint16_t Touchscreen::spiTransferU16(uint16_t data) {
  uint16_t result = 0;
  HAL_SPI_TransmitReceive(touchSpi, (uint8_t*)&data, (uint8_t*)&result, 2, DELAY);
  return result;
}

uint16_t Touchscreen::readX() {
  update();
  return currentX;
}

uint16_t Touchscreen::readY() {
  update();
  return currentY;
}