#pragma once

#include <cstdint>

#include "display.hpp"

namespace Render {

struct LCDColor {
public:
  uint16_t packed;

  static LCDColor fromRgb8(uint8_t r, uint8_t g, uint8_t b) {
    return LCDColor{.packed = Display::rgb8To565(r, g, b)};
  }

  uint16_t as565() {
    return packed;
  }

  std::tuple<uint8_t, uint8_t, uint8_t> asRgb8() {
    return Display::rgb565To8(packed);
  }
};

const LCDColor RED = LCDColor::fromRgb8(255, 0, 0);
const LCDColor GREEN = LCDColor::fromRgb8(0, 255, 0);
const LCDColor BLUE = LCDColor::fromRgb8(0, 0, 255);
const LCDColor WHITE = LCDColor::fromRgb8(255, 255, 255);
const LCDColor BLACK = LCDColor::fromRgb8(0, 0, 0);

enum LCDTextureWrap { Fill, Tile };
enum LCDTextureBlend { None, Additive };

struct LCDTexture {
public:
  const uint16_t* data;
  uint16_t width;
  uint16_t height;
  LCDTextureWrap wrap;
  LCDTextureBlend blend;

  static LCDTexture fromColors(const LCDColor data[], uint16_t width, uint16_t height, LCDTextureWrap wrap, LCDTextureBlend blend) {
    return LCDTexture{.data = (uint16_t*)data, .width = width, .height = height, .wrap = wrap, .blend = blend};
  }

  static LCDTexture from565(const uint16_t data[], uint16_t width, uint16_t height, LCDTextureWrap wrap, LCDTextureBlend blend) {
    return LCDTexture{.data = data, .width = width, .height = height, .wrap = wrap, .blend = blend};
  }
};

class LCDRenderer {
private:
  LCDColor currentColor = LCDColor::fromRgb8(255, 255, 255);
  std::optional<LCDTexture> currentTexture = std::nullopt;
  Display::Controller* lcd;

public:
  LCDRenderer(Display::Controller* lcd);

  void setColor(LCDColor color);
  void setTexture(std::optional<LCDTexture> texture);
  void setRotation(Display::Rotation rotation);
  void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
  void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void drawCircleOutline(uint16_t x, uint16_t y, uint16_t radius, uint8_t thickness = 1);
};

}; // namespace Render