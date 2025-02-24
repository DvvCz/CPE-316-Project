#pragma once

/*
  This is a library for the MSP2807 LCD which uses the ILI9341 LCD driver
  It doesn't handle the touch functionality of the board which is handled separately.
*/

#include "main.h"

#include <optional>
#include <tuple>

namespace Display {
struct LCDPins {
  GPIO_TypeDef* mosiPort;
  uint16_t mosiPin;

  GPIO_TypeDef* misoPort;
  uint16_t misoPin;

  GPIO_TypeDef* sClkPort;
  uint16_t sClkPin;

  std::optional<GPIO_TypeDef*> csPort;
  std::optional<uint16_t> csPin;

  GPIO_TypeDef* resetPort;
  uint16_t resetPin;

  GPIO_TypeDef* dcPort;
  uint16_t dcPin;
};

const uint16_t WIDTH = 240;
const uint16_t HEIGHT = 320;

enum DCMode { Data = 1, Command = 0 };
enum CSMode { Active = 0, Idle = 1 };

enum Rotation {
  /* 0 degrees */
  Landscape = 0,

  /* 90 degrees */
  Portrait = 1,

  /* 180 degrees */
  ReverseLandscape = 2,

  /* 270 degrees */
  ReversePortrait = 3
};

#define ILI9341_SOFTRESET 0x01
#define ILI9341_SLEEPIN 0x10
#define ILI9341_SLEEPOUT 0x11
#define ILI9341_NORMALDISP 0x13
#define ILI9341_INVERTOFF 0x20
#define ILI9341_INVERTON 0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPLAYOFF 0x28
#define ILI9341_DISPLAYON 0x29
#define ILI9341_COLADDRSET 0x2A
#define ILI9341_PAGEADDRSET 0x2B
#define ILI9341_MEMORYWRITE 0x2C
#define ILI9341_MEMORYACCESS 0x36
#define ILI9341_PIXELFORMAT 0x3A
#define ILI9341_RGBSIGNAL 0xB0
#define ILI9341_FRAMECONTROL 0xB1
#define ILI9341_INVERSIONCONRTOL 0xB4
#define ILI9341_DISPLAYFUNC 0xB6
#define ILI9341_ENTRYMODE 0xB7
#define ILI9341_POWERCONTROL1 0xC0
#define ILI9341_POWERCONTROL2 0xC1
#define ILI9341_VCOMCONTROL1 0xC5
#define ILI9341_VCOMCONTROL2 0xC7
#define ILI9341_POWERCONTROLB 0xCF
#define ILI9341_POWERCONTROLA 0xCB
#define ILI9341_UNDEFINE0 0xE0
#define ILI9341_UNDEFINE1 0xE1
#define ILI9341_DRIVERTIMINGA 0xE8
#define ILI9341_DRIVERTIMINGB 0xEA
#define ILI9341_POWERONSEQ 0xED
#define ILI9341_ENABLE3G 0xF2
#define ILI9341_INTERFACECONTROL 0xF6
#define ILI9341_RUMPRATIO 0xF7
#define ILI9341_MEMCONTROL 0x36
#define ILI9341_MADCTL 0x36

#define ILI9341_MADCTL_MY 0x80
#define ILI9341_MADCTL_MX 0x40
#define ILI9341_MADCTL_MV 0x20
#define ILI9341_MADCTL_ML 0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH 0x04

uint16_t rgb8To565(const uint8_t r, const uint8_t g, const uint8_t b);
std::tuple<uint8_t, uint8_t, uint8_t> rgb565To8(const uint16_t color);
uint16_t mix(const uint16_t color1, const uint16_t color2, const float ratio);

class Controller {
private:
  SPI_HandleTypeDef* lcdSpi;
  SPI_HandleTypeDef* touchSpi;

  void setDCMode(const DCMode mode);
  void setCSMode(const CSMode mode);

  void spiWrite(const uint8_t* data, const uint16_t size);
  void spiWriteU8(const uint8_t data);
  void spiWriteU16(const uint16_t data);

  void spiRead(uint8_t* data, uint16_t size);
  [[nodiscard]] uint8_t spiReadU8();
  [[nodiscard]] uint16_t spiReadU16();

public:
  Controller(SPI_HandleTypeDef* lcdSpi, const LCDPins lcdPins);
  LCDPins lcdPins;

  void init();
  void reset();

  void writeCommandU8(const uint8_t cmd);
  void writeCommandU16(const uint16_t cmd);

  void writeDataU8(const uint8_t data);
  void writeDataU16(const uint16_t data);

  void setAddrWindow(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1);
  void setRotation(const Rotation r);
  void setInverted(const bool inverted);

  void drawRect(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t color);

  void drawRectTextured(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t* colors);

  void drawRectTexturedStretch(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t* colors, const uint16_t textureWidth, const uint16_t textureHeight);
  void drawRectTexturedTiling(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t* colors, const uint16_t textureWidth, const uint16_t textureHeight);

  void drawRectTexturedStretchMix(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t* colors, const uint16_t textureWidth, const uint16_t textureHeight, const uint16_t color);
  void drawRectTexturedTilingMix(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const uint16_t* colors, const uint16_t textureWidth, const uint16_t textureHeight, const uint16_t color);
};

} // namespace Display