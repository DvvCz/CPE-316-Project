#include "render.hpp"

Render::LCDRenderer::LCDRenderer(Display::Controller* lcd) {
  this->lcd = lcd;
}

void Render::LCDRenderer::drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  if (currentTexture.has_value()) {
    auto currentTexture = this->currentTexture.value();
    switch (currentTexture.wrap) {
    case LCDTextureWrap::Fill:
      lcd->drawRectTexturedStretchMix(x, y, width, height, currentTexture.data, currentTexture.width,
                                   currentTexture.height, currentColor.as565());
      break;
    case LCDTextureWrap::Tile:
      lcd->drawRectTexturedTilingMix(x, y, width, height, currentTexture.data, currentTexture.width,
                                  currentTexture.height, currentColor.as565());
      break;
    }
  } else {
    lcd->drawRect(x, y, width, height, currentColor.as565());
  }
}

void Render::LCDRenderer::drawDiagonalLine(uint16_t x, uint16_t y, uint16_t size, bool topLeftToBottomRight, Render::LCDColor color) {
    if (topLeftToBottomRight) {
        //you can change if you want to draw diagonal going right or left with this bool
        for (uint16_t i = 0; i < size; ++i) {
            lcd->drawRect(x + i, y + i, 1, 1, color.as565());  // Draw each pixel along the diagonal individually
        }
    } else {
        // Draw top-right to bottom-left diagonal
        for (uint16_t i = 0; i < size; ++i) {
            lcd->drawRect(x + size - i - 1, y + i, 1, 1, color.as565());  // Draw each pixel along the diagonal individually
        }
    }
}

void Render::LCDRenderer::drawX(uint16_t x, uint16_t y, uint16_t size, Render::LCDColor color) {
    // draw first diagonal left to right
    drawDiagonalLine(x, y, size, true, color);

    // draw second diagonal right to left
    drawDiagonalLine(x + size - 1, y, size, false, color);
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

void Render::LCDRenderer::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) { //this function only works for horizontal and vertical lines
    //horizontal lines
    if (y0 == y1) {
        for (uint16_t x = x0; x <= x1; x++) {
            lcd->drawRect(x, y0, 1, 1, color);  // Draw each pixel in the horizontal line
        }
    }
    // vertical line
    else if (x0 == x1) {
        for (uint16_t y = y0; y <= y1; y++) {
            lcd->drawRect(x0, y, 1, 1, color);  // Draw each pixel in the vertical line
        }
    }
}


void Render::LCDRenderer::drawGrid() { //maybe can change this to draw grid a bit differently if it looks weird
    
    //draw horizontal lines
    drawLine(0, 80, 320, 80, currentColor.as565());
    drawLine(0, 160, 320, 160, currentColor.as565());



    // draw vertical lines, wont be exactly symmetrical because it is 320 pixels in x direction which is not divisible by 3 but likely wont be noticable
    drawLine(107, 0, 107, 240, currentColor.as565());
    drawLine(214, 0, 214, 240. currentColor.as565());
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