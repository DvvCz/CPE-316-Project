
#include "touchscreen.hpp"

#include "stm32l4xx_hal.h"

// Touchscreen class definition
Touchscreen::Touchscreen(SPI_HandleTypeDef* spi, TouchPins pins) {
    touchSpi = spi;
    touchPins = pins;
}

// Initializes touchscreen (setup SPI)
void Touchscreen::init() {
    // Setup SPI settings or any other initial configuration for the touchscreen

    //NEED SOMETHING HERE
}

void Touchscreen::setCallback(TouchCallback callback) {
    callback = callback;
}

// Reads data from touchscreen over SPI
uint8_t Touchscreen::spiReadU8() {
    uint8_t data;
    HAL_SPI_Receive(touchSpi, &data, 1, HAL_MAX_DELAY);
    return data;
}

uint16_t Touchscreen::readTouchX() {
    uint8_t cmd = 0x90;  // Command to read X coordinate MAYBE WILL BE DIFFERENT DEPENDING ON THE TOUCHSCREEN CONTROLLER USED
    uint8_t xValue[2];

    // Activate Chip Select (CS)
    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_RESET);

    // Send command to read X
    spiWriteU8(cmd);

    // Read X value (2 bytes)
    spiRead(touchPins, xValue, 2);

    // Deactivate CS
    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_SET);

    // Combine bytes to form 12-bit X value
    uint16_t x = (xValue[0] << 8) | xValue[1];
    return x;
}

uint16_t Touchscreen::readTouchY() {

    uint8_t cmd = 0xD0;  // Command to read Y coordinate MAYBE WILL BE DIFFERENT BASED ON TOUCHSCREEN CONTROLLER
    uint8_t yValue[2];

    // Activate Chip Select (CS)
    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_RESET);

    // Send command to read Y
    spiWriteU8(cmd);

    // Read Y value (2 bytes)
    spiRead(touchPins, yValue, 2);

    // Deactivate CS
    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_SET);

    // Combine bytes to form 12-bit Y value
    uint16_t y = (yValue[0] << 8) | yValue[1];
    return y;
}

// Function to check if touchscreen is touched (IRQ pin)
bool Touchscreen::isTouched() {
    return HAL_GPIO_ReadPin(touchPins.irqPort, touchPins.irqPin) == GPIO_PIN_RESET;
}

void Touchscreen::irqHook() {
    // Check if IRQ pin is low (indicating touch)
    if (isTouched()) {
        if (callback.has_value()) {
            uint16_t x = readTouchX();
            uint16_t y = readTouchY();
            (callback.value())(x, y);
        }
    }
}

// Helper functions to communicate via SPI
void Touchscreen::spiWriteU8(uint8_t data) {
    HAL_SPI_Transmit(touchSpi, &data, 1, HAL_MAX_DELAY);
}

void Touchscreen::spiRead(TouchPins touchPins, uint8_t* data, uint16_t size) {
    HAL_SPI_Receive(touchSpi, data, size, HAL_MAX_DELAY);
}


