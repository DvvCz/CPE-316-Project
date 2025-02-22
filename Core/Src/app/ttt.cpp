#include "ttt.hpp"

#include "../lib/ttt.hpp"
#include "../lib/display.hpp"
#include "../lib/uart.hpp"

#include "main.h"

void App::init() {
  // auto b = new Board();
  // b->setTile(0, 0, tile_state_t::o);
  // b->setTile(0, 1, tile_state_t::x);
  // b->setTile(0, 2, tile_state_t::o);
  // b->setTile(1, 0, tile_state_t::x);
  // b->setTile(1, 1, tile_state_t::x);
  // b->setTile(1, 2, tile_state_t::o);
  // b->setTile(2, 0, tile_state_t::x);
  // b->setTile(2, 1, tile_state_t::o);
  // b->setTile(2, 2, tile_state_t::x);

  /* clang-format off */
  auto lcd = Display::Controller(&hspi2, {
    .mosiPort = LCD_MOSI_GPIO_Port,
    .mosiPin = LCD_MOSI_Pin,

    .sClkPort = LCD_SCK_GPIO_Port,
    .sClkPin = LCD_SCK_Pin,

    .resetPort = LCD_RESET_GPIO_Port,
    .resetPin = LCD_RESET_Pin,

    .dcPort = LCD_DC_GPIO_Port,
    .dcPin = LCD_DC_Pin
  });
  /* clang-format off */

  lcd.init();

  uint16_t width = Display::WIDTH;
  uint16_t height = Display::HEIGHT;

  uint16_t R = Display::rgb8To565(255, 0, 0);
  uint16_t B = Display::rgb8To565(0, 0, 0);

  uint16_t checkeredTexture[64] = {
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,

    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R
  };

  lcd.drawRectTexturedTiling(0, 0, width, height, checkeredTexture, 8, 8);

  while (true) {
    lcd.setRotation(Display::Landscape);

    lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 0, 0));
    lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 255, 0));
    lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 0, 255));

    lcd.setRotation(Display::Portrait);

    lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 0, 0));
    lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 255, 0));
    lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 0, 255));

    lcd.setRotation(Display::ReverseLandscape);

    lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 255, 255));
    lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 0, 255));
    lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 255, 0));

    lcd.setRotation(Display::ReversePortrait);

    lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 255, 255));
    lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 0, 255));
    lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 255, 0));
  }
}