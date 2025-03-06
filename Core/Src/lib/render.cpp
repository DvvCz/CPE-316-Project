#include "render.hpp"

#include "uart.hpp"

Render::LCDRenderer::LCDRenderer(Display::Controller* lcd) {
  this->lcd = lcd;
}

void Render::LCDRenderer::drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  if (currentTexture.has_value()) {
    auto currentTexture = this->currentTexture.value();
    switch (currentTexture.wrap) {
    case LCDTextureWrap::Fill:
      lcd->drawRectTextured(x, y, width, height, currentTexture.data, currentTexture.width,
                                   currentTexture.height, Display::TexturePosition::stretching);
      break;
    case LCDTextureWrap::Tile:
      lcd->drawRectTextured(x, y, width, height, currentTexture.data, currentTexture.width,
                                  currentTexture.height, Display::TexturePosition::tiling);
      break;
    }
  } else {
    lcd->drawRect(x, y, width, height, currentColor.as565());
  }
}

/*
    TODO: Could possibly split this into a cleaner rounded rectangle corner fn.
    Then just use that 4 times with a certain fidelity.
*/
void drawSingleCircleOutline(Display::Controller* lcd, uint16_t x, uint16_t y, uint16_t radius, Render::LCDColor currentColor) {
    int x0 = 0;
    int y0 = radius;
    int d = 3 - 2 * radius;

    while (y0 >= x0) {
        lcd->drawRect(x + x0, y + y0, 1, 1, currentColor.as565());
        lcd->drawRect(x + x0, y - y0, 1, 1, currentColor.as565());
        lcd->drawRect(x - x0, y + y0, 1, 1, currentColor.as565());
        lcd->drawRect(x - x0, y - y0, 1, 1, currentColor.as565());
        lcd->drawRect(x + y0, y + x0, 1, 1, currentColor.as565());
        lcd->drawRect(x + y0, y - x0, 1, 1, currentColor.as565());
        lcd->drawRect(x - y0, y + x0, 1, 1, currentColor.as565());
        lcd->drawRect(x - y0, y - x0, 1, 1, currentColor.as565());

        if (d < 0) {
        d += 4 * x0 + 6;
        } else {
        d += 4 * (x0 - y0) + 10;
        y0--;
        }
        x0++;
    }
}

void Render::LCDRenderer::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) { //this function only works for horizontal and vertical lines
    if (y0 == y1) { // Horizontal
      drawRect(x0, y0, x1 - x0, 1);
    } else if (x0 == x1) { // Vertical
      drawRect(x0, y0, 1, y1 - y0);
    } else {
      // Bresenham's line algorithm for diagonal lines
      int dx = abs(x1 - x0);
      int dy = abs(y1 - y0);
      int sx = x0 < x1 ? 1 : -1;
      int sy = y0 < y1 ? 1 : -1;
      int err = dx - dy;

      while (true) {
        lcd->drawRect(x0, y0, 1, 1, currentColor.as565());
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
          err -= dy;
          x0 += sx;
        }
        if (e2 < dx) {
          err += dx;
          y0 += sy;
        }
      }
    }
}

void Render::LCDRenderer::drawCircleOutline(uint16_t x, uint16_t y, uint16_t radius, uint8_t thickness) {
    for (uint8_t i = 0; i < thickness; i++) {
        drawSingleCircleOutline(lcd, x, y, radius + i, currentColor);
    }
}

void Render::LCDRenderer::setColor(Render::LCDColor color) {
  currentColor = color;
}

void Render::LCDRenderer::setTexture(std::optional<Render::LCDTexture> texture) {
  currentTexture = texture;
}

void Render::LCDRenderer::setRotation(Display::Rotation rotation) {
  lcd->setRotation(rotation);
}