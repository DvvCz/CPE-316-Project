#include "display.hpp"

#include "uart.hpp"
#include <functional>

uint16_t Display::TexturePosition::stretching(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t textureWidth, uint16_t textureHeight) {
  return (x * textureWidth) / w + (y * textureHeight) / h * textureWidth;
};

uint16_t Display::TexturePosition::tiling(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t textureWidth, uint16_t textureHeight) {
  return (x % textureWidth) + (y % textureHeight) * textureWidth;
};

uint16_t Display::TextureColor::identity(const uint16_t* colors, uint16_t index) {
  return colors[index];
};

Display::Controller::Controller(SPI_HandleTypeDef* spi, LCDPins pins) {
  lcdSpi = spi;
  lcdPins = pins;
}

void Display::Controller::spiWrite(const uint8_t* data, const uint16_t size) {
  HAL_SPI_Transmit(this->lcdSpi, data, size, HAL_MAX_DELAY);
}

void Display::Controller::spiWriteU8(const uint8_t data) {
  this->spiWrite(&data, 1);
}

void Display::Controller::spiWriteU16(const uint16_t data) {
  this->spiWriteU8(data >> 8);   // hi
  this->spiWriteU8(data & 0xFF); // lo
}

void Display::Controller::spiRead(uint8_t* data, uint16_t size) {
  HAL_SPI_Receive(this->lcdSpi, data, size, HAL_MAX_DELAY);
}

uint8_t Display::Controller::spiReadU8() {
  uint8_t out;
  this->spiRead(&out, 1);
  return out;
}

uint16_t Display::Controller::spiReadU16() {
  uint16_t out;
  this->spiRead((uint8_t*)&out, 2);
  return out;
}

void Display::Controller::reset() {
  HAL_GPIO_WritePin(this->lcdPins.resetPort, this->lcdPins.resetPin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(this->lcdPins.resetPort, this->lcdPins.resetPin, GPIO_PIN_SET);
}

void Display::Controller::setDCMode(const DCMode mode) {
  if (mode == DCMode::Data) {
    HAL_GPIO_WritePin(this->lcdPins.dcPort, this->lcdPins.dcPin, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(this->lcdPins.dcPort, this->lcdPins.dcPin, GPIO_PIN_RESET);
  }
}

void Display::Controller::setCSMode(const CSMode mode) {
  auto csPort = this->lcdPins.csPort;
  auto csPin = this->lcdPins.csPin;

  if (!csPort.has_value() || !csPin.has_value()) {
    /* Using hardware CS (NSS) */
    return;
  }

  if (mode == CSMode::Active) {
    HAL_GPIO_WritePin(csPort.value(), csPin.value(), GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(csPort.value(), csPin.value(), GPIO_PIN_SET);
  }
}

void Display::Controller::writeCommandU8(const uint8_t command) {
  this->setCSMode(CSMode::Active);

  this->setDCMode(DCMode::Command);
  this->spiWriteU8(command);

  this->setCSMode(CSMode::Idle);
}

void Display::Controller::writeCommandU16(const uint16_t command) {
  this->setCSMode(CSMode::Active);

  this->setDCMode(DCMode::Command);
  this->spiWriteU16(command);

  this->setCSMode(CSMode::Idle);
}

void Display::Controller::writeDataU8(const uint8_t data) {
  this->setCSMode(CSMode::Active);

  this->setDCMode(DCMode::Data);
  this->spiWriteU8(data);

  this->setCSMode(CSMode::Idle);
}

void Display::Controller::writeDataU16(const uint16_t data) {
  this->setCSMode(CSMode::Active);

  this->setDCMode(DCMode::Data);
  this->spiWriteU16(data);

  this->setCSMode(CSMode::Idle);
}

void Display::Controller::setAddrWindow(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1) {
  this->writeCommandU16(ILI9341_COLADDRSET);
  this->writeDataU16(x0);
  this->writeDataU16(x1);

  this->writeCommandU16(ILI9341_PAGEADDRSET);
  this->writeDataU16(y0);
  this->writeDataU16(y1);
}

void Display::Controller::drawRectTextured( /* clang-format off */
  const uint16_t x,
  const uint16_t y,
  const uint16_t w,
  const uint16_t h,
  const uint16_t* colors,
  const std::optional<uint16_t> textureWidth,
  const std::optional<uint16_t> textureHeight,
  const Display::TexturePosition::Resolver positionResolver,
  const Display::TextureColor::Resolver colorResolver
) { /* clang-format on */
  this->setAddrWindow(x, y, x + w - 1, y + h - 1);
  this->writeCommandU16(ILI9341_MEMORYWRITE);

  auto texWidth = textureWidth.value_or(w);
  auto texHeight = textureHeight.value_or(h);

  for (uint16_t y = 0; y < h; y++) {
    for (uint16_t x = 0; x < w; x++) {
      uint16_t texIndex = positionResolver(x, y, w, h, texWidth, texHeight);
      uint16_t texColor = colorResolver(colors, texIndex);

      this->writeDataU16(texColor);
    }
  }
}

void Display::Controller::drawRect( /* clang-format off */
  const uint16_t x,
  const uint16_t y,
  const uint16_t w,
  const uint16_t h,
  const uint16_t color
) { /* clang-format on */
  drawRectTextured(x, y, w, h, &color, 1, 1, Display::TexturePosition::tiling);
}

void Display::Controller::setRotation(const Rotation r) {
  uint8_t rotationCode;
  switch (r) {
  case Rotation::Landscape: // 0
    rotationCode = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR;
    break;
  case Rotation::Portrait: // 90
    rotationCode = ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR;
    break;
  case Rotation::ReverseLandscape: // 180
    rotationCode = ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR;
    break;
  case Rotation::ReversePortrait: // 270
    rotationCode = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR;
    break;
  }

  this->setCSMode(CSMode::Active);

  this->writeCommandU8(ILI9341_MADCTL);
  this->writeDataU8(rotationCode);

  this->setCSMode(CSMode::Idle);
}

void Display::Controller::setInverted(const bool inverted) {
  this->setCSMode(CSMode::Active);

  this->writeDataU8(inverted ? ILI9341_INVERTON : ILI9341_INVERTOFF);

  this->setCSMode(CSMode::Idle);
}

// Makes it easier to define commands by automatically calculating passed
// argument count.
#define CMD(name, ...) name, (sizeof((int[]){__VA_ARGS__}) / sizeof(int)) __VA_OPT__(, ) __VA_ARGS__

// Hardcoded delay between commands.
// Not actually transmitted
#define DELAY_CMD 0x7F

// Format: CMD(cmd, data1, data2, ...)
uint8_t initCommands[] = {
    CMD(ILI9341_SOFTRESET),
    CMD(DELAY_CMD, 50),
    CMD(ILI9341_DISPLAYOFF),
    CMD(ILI9341_INTERFACECONTROL, 0x01, 0x01, 0x00),
    CMD(ILI9341_POWERCONTROLB, 0x00, 0x81, 0x30),
    CMD(ILI9341_POWERONSEQ, 0x64, 0x03, 0x12, 0x81),
    CMD(ILI9341_DRIVERTIMINGA, 0x85, 0x10, 0x78),
    CMD(ILI9341_POWERCONTROLA, 0x39, 0x2C, 0x00, 0x34, 0x02),
    CMD(ILI9341_RUMPRATIO, 0x20),
    CMD(ILI9341_DRIVERTIMINGB, 0x00, 0x00),
    CMD(ILI9341_RGBSIGNAL, 0x00),
    CMD(ILI9341_INVERSIONCONRTOL, 0x00),
    CMD(ILI9341_POWERCONTROL1, 0x21),
    CMD(ILI9341_POWERCONTROL2, 0x11),
    CMD(ILI9341_VCOMCONTROL1, 0x3F, 0x3C),
    CMD(ILI9341_VCOMCONTROL2, 0xB5),
    CMD(ILI9341_MEMCONTROL, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR),
    CMD(ILI9341_PIXELFORMAT, 0x55),
    CMD(ILI9341_FRAMECONTROL, 0x00, 0x1B),
    CMD(ILI9341_MEMORYACCESS, 0x48),
    CMD(ILI9341_ENABLE3G, 0x00),
    CMD(ILI9341_GAMMASET, 0x01),
    CMD(ILI9341_UNDEFINE0, 0x0f, 0x26, 0x24, 0x0b, 0x0e, 0x09, 0x54, 0xa8, 0x46, 0x0c, 0x17, 0x09, 0x0f, 0x07, 0x00),
    CMD(ILI9341_UNDEFINE1, 0x00, 0x19, 0x1b, 0x04, 0x10, 0x07, 0x2a, 0x47, 0x39, 0x03, 0x06, 0x06, 0x30, 0x38, 0x0f),
    CMD(ILI9341_ENTRYMODE, 0x07),
    CMD(ILI9341_SLEEPOUT),
    CMD(DELAY_CMD, 150),
    CMD(ILI9341_DISPLAYON)};

void Display::Controller::init() {
  this->reset();

  HAL_Delay(200);

  int len = sizeof(initCommands) / sizeof(uint8_t);
  int ptr = 0;

  while (ptr < len) {
    uint8_t cmd = initCommands[ptr++];
    uint8_t dataCount = initCommands[ptr++];

    if (cmd == DELAY_CMD) {
      HAL_Delay(initCommands[ptr++]);
      continue;
    }

    this->writeCommandU16(cmd);
    for (int i = 0; i < dataCount; i++) {
      this->writeDataU8(initCommands[ptr++]);
    }
  }

  this->setRotation(Rotation::Landscape);
  this->setInverted(false);
}

uint16_t Display::rgb8To565(const uint8_t r, const uint8_t g, const uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

std::tuple<uint8_t, uint8_t, uint8_t> Display::rgb565To8(const uint16_t color) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  return std::make_tuple(r, g, b);
}