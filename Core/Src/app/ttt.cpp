#include "ttt.hpp"

#include "../lib/ttt.hpp"
#include "../lib/display.hpp"
#include "../lib/uart.hpp"
#include "../lib/render.hpp"

#include "main.h"

#define delta_x 27
#define delta_y 20
#define x_size 45

#define circ_x 53
#define circ_y 40
#define circ_radius 20

Display::Controller lcd(&hspi2, {
    .mosiPort = LCD_MOSI_GPIO_Port,
    .mosiPin = LCD_MOSI_Pin,
    .sClkPort = LCD_SCK_GPIO_Port,
    .sClkPin = LCD_SCK_Pin,
    .resetPort = LCD_RESET_GPIO_Port,
    .resetPin = LCD_RESET_Pin,
    .dcPort = LCD_DC_GPIO_Port,
    .dcPin = LCD_DC_Pin
});

Render::LCDRenderer render(&lcd);

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
  // auto lcd = Display::Controller(&hspi2, {
  //   .mosiPort = LCD_MOSI_GPIO_Port,
  //   .mosiPin = LCD_MOSI_Pin,

  //   .sClkPort = LCD_SCK_GPIO_Port,
  //   .sClkPin = LCD_SCK_Pin,

  //   .resetPort = LCD_RESET_GPIO_Port,
  //   .resetPin = LCD_RESET_Pin,

  //   .dcPort = LCD_DC_GPIO_Port,
  //   .dcPin = LCD_DC_Pin
  // });
  /* clang-format off */

  lcd.init();

  //auto render = Render::LCDRenderer(&lcd);

  auto R = Render::WHITE;
  auto B = Render::BLACK;

  Render::LCDColor checkeredTexture[] = {
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,
    R, R, R, R, B, B, B, B,

    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R,
    B, B, B, B, R, R, R, R
  };

  Render::LCDTexture texture = Render::LCDTexture::fromColors(checkeredTexture, 8, 8, Render::LCDTextureWrap::Tile, Render::LCDTextureBlend::Additive);

  render.setRotation(Display::Landscape);
  // render.setTexture(texture);

  auto red = Render::LCDColor::fromRgb8(255, 0, 0);
  auto blue = Render::LCDColor::fromRgb8(0, 0, 255);
  // auto purple = blue.mix(red, 0.5f);

  int i = 0;

  render.setColor(blue);

  render.drawCircleOutline(Display::WIDTH / 2, Display::HEIGHT / 2, 1, 200);

  // while (true) {
  //   auto rectW = 25;
  //   auto rectH = 25;

  //   auto centerW = Display::WIDTH / 2;
  //   auto centerH = Display::HEIGHT / 2;

  //   render.setColor(Render::LCDColor::fromRgb8(i, 0, 0));
  //   render.drawRect(centerW - rectW / 2, centerH - rectH / 2, rectW, rectH);

  //   i = (i + 5) % 255;
  // }


  // uint16_t width = Display::WIDTH;
  // uint16_t height = Display::HEIGHT;

  // uint16_t R = Display::rgb8To565(255, 0, 0);
  // uint16_t B = Display::rgb8To565(0, 0, 0);

  // uint16_t checkeredTexture[64] = {
  //   R, R, R, R, B, B, B, B,
  //   R, R, R, R, B, B, B, B,
  //   R, R, R, R, B, B, B, B,
  //   R, R, R, R, B, B, B, B,

  //   B, B, B, B, R, R, R, R,
  //   B, B, B, B, R, R, R, R,
  //   B, B, B, B, R, R, R, R,
  //   B, B, B, B, R, R, R, R
  // };

  // lcd.drawRectTexturedTiling(0, 0, width, height, checkeredTexture, 8, 8);

  // while (true) {
  //   lcd.setRotation(Display::Landscape);

  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 0, 0));
  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 255, 0));
  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 0, 255));

  //   lcd.setRotation(Display::Portrait);

  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 0, 0));
  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 255, 0));
  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 0, 255));

  //   lcd.setRotation(Display::ReverseLandscape);

  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(0, 255, 255));
  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 0, 255));
  //   lcd.drawRect(0, 0, width, height, Display::rgb8To565(255, 255, 0));

  //   lcd.setRotation(Display::ReversePortrait);

  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(0, 255, 255));
  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 0, 255));
  //   lcd.drawRect(0, 0, height, width, Display::rgb8To565(255, 255, 0));
  // }
}

Board::Board() {
    // Initialize the board with empty tiles
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            grid[i][j] = TileState::Empty;
        }
    }
    render.drawGrid();

}

void Board::setTile(int row, int col, TileState state) {

    //update grid in software, this grid can be used to check for the winner
    if (row >= 0 && row < 3 && col >= 0 && col < 3) {  // Check bounds
        if (grid[row][col] == TileState::Empty) {  // Ensure the tile isn't already occupied
            grid[row][col] = state;
            writeTile(row, col, state);
        }
    }
}

void Board::isFull(){
    for (int i = 0; i < 3; i++){
      for (int j = 0; i < 3; i++){
        if (grid[i][j] == TileState::Empty){
          return false;
        }
      }
    }
    return true;
}

TileState Board::checkWinner(){
    for (int i = 0; i < 3; i++) {
            if (grid[i][0] != TileState::Empty && grid[i][0] == grid[i][1] && grid[i][1] == grid[i][2]) {
                return grid[i][0]; // Return the winner (X or O)
            }
        }

        // Check columns for a winner
        for (int i = 0; i < 3; i++) {
            if (grid[0][i] != TileState::Empty && grid[0][i] == grid[1][i] && grid[1][i] == grid[2][i]) {
                return grid[0][i]; // Return the winner (X or O)
            }
        }

        // Check top-left to bottom-right diagonal
        if (grid[0][0] != TileState::Empty && grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2]) {
            return grid[0][0]; // Return the winner (X or O)
        }

        // Check top-right to bottom-left diagonal
        if (grid[0][2] != TileState::Empty && grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0]) {
            return grid[0][2]; // Return the winner (X or O)
        }

        // No winner found
        return TileState::Empty; // No winner yet

}


void Board::writeTile(int row, int col, TileState state){ //probably call this function in set tile if the tilestate is valid to write to 


    if (state == TileState::X){ //will probably have to mess around with the x placements to make it look good 

        switch (row*3 + col){ //row*3 + col is so that i can determine what row and col are within 1 switch statement
            case 0:{ //0,0
              render.drawX(delta_x, delta_y, x_size, currentColor.as565());
              break;
            }
            case 1:{ //0,1
              render.drawX(107 + delta_x, delta_y, x_size, currentColor.as565());
              break;
            }
            case 2:{ //0,2
              render.drawX(214 + delta_x, delta_y, x_size, currentColor.as565());
              break;
            }
            case 3:{ //1,0
              render.drawX(delta_x, 80 + delta_y, x_size, currentColor.as565());
              break;
            }
            case 4:{ //1,1
              render.drawX(107 + delta_x, 80 + delta_y, x_size, currentColor.as565());
              break;
            }
            case 5:{ //1,2
              render.drawX(214 + delta_x, 80 + delta_y, x_size, currentColor.as565());
              break;
            }
            case 6:{ //2,0
              render.drawX(delta_x, 160 + delta_y, x_size, currentColor.as565());
              break;
            }
            case 7:{ //2,1
              render.drawX(107 + delta_x, 160 + delta_y, x_size, currentColor.as565());
              break;
            }
            case 8:{ //2,2
              render.drawX(214 + delta_x, 160 + delta_y, x_size, currentColor.as565());
              break;
            }
        }

      }
      else if (state == TileState::O){
          switch (row*3 + col){ //row*3 + col is so that i can determine what row and col are within 1 switch statement
            case 0:{ //0,0
                render.drawCircleOutline(circ_x, circ_y, circ_radius);
              break;
            }
            case 1:{ //0,1
              render.drawCircleOutline(107 + circ_x, circ_y, circ_radius);
              break;
            }
            case 2:{ //0,2
              render.drawCircleOutline(214 + circ_x, circ_y, circ_radius);
              break;
            }
            case 3:{ //1,0
              render.drawCircleOutline(circ_x, 80 + circ_y, circ_radius);
              break;
            }
            case 4:{ //1,1
              render.drawCircleOutline(107 + circ_x, 80 + circ_y, circ_radius);
              break;
            }
            case 5:{ //1,2
              render.drawCircleOutline(214 + circ_x, 80 + circ_y, circ_radius);
              break;
            }
            case 6:{ //2,0
              render.drawCircleOutline(circ_x, 160 + circ_y, circ_radius);
              break;
            }
            case 7:{ //2,1
              render.drawCircleOutline(107 + circ_x, 160 + circ_y, circ_radius);
              break;
            }
            case 8:{ //2,2
              render.drawCircleOutline(214 + circ_x, 160 + circ_y, circ_radius);
              break;
            }
        }
      }
}
    

TileState Board::getTile(int row, int col){
    return grid[row][column];
}