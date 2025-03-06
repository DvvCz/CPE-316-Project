
#include "touchscreen.hpp"

#include "stm32l4xx_hal.h"

#include "uart.hpp"

// Touchscreen class definition
Touchscreen::Touchscreen(SPI_HandleTypeDef* spi, TouchPins pins) {
    touchSpi = spi;
    touchPins = pins;
}

// Initializes touchscreen (setup SPI)
void Touchscreen::init() {
    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(touchPins.dinPort, touchPins.dinPin, GPIO_PIN_SET);
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
        uint16_t x = readX();
        uint16_t y = readY();
        (callback.value())(x, y);
    }
}

void Touchscreen::writeU8(uint8_t data) {
    uint8_t val = 0x80;
    while (val) {
        if (data & val) {
            HAL_GPIO_WritePin(touchPins.dinPort, touchPins.dinPin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(touchPins.dinPort, touchPins.dinPin, GPIO_PIN_RESET);
        }

        HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_SET);

        val >>= 1;
    }
}

static void delayMicroseconds(uint32_t us) {
    // Configured for 80MHz
    uint32_t cycles = us * 80;
    while (cycles--) { __NOP(); }
}

uint16_t Touchscreen::readADC(uint8_t cmd) {
    uint16_t val = 0;

    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(touchPins.dinPort, touchPins.dinPin, GPIO_PIN_RESET);

    writeU8(cmd);
    delayMicroseconds(6);

    HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);
    delayMicroseconds(1);
    HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);

    // for (int i = 0; i < 16; i++) {
    //     HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);
    //     HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_SET);

    //     if (HAL_GPIO_ReadPin(touchPins.doPort, touchPins.doPin) == GPIO_PIN_SET) {
    //         val |= 1 << (15 - i);
    //     }
    // }

    // val >>= 4; // Discard last 4 bits

    for (int i = 0; i < 12; i++) {
        HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(touchPins.clkPort, touchPins.clkPin, GPIO_PIN_SET);

        if (HAL_GPIO_ReadPin(touchPins.doPort, touchPins.doPin) == GPIO_PIN_SET) {
            val |= 1 << (11 - i);
        }
    }

    HAL_GPIO_WritePin(touchPins.csPort, touchPins.csPin, GPIO_PIN_SET);

    return val;
}

#define SAMPLES 15

uint16_t Touchscreen::readChannel(uint8_t channel) {
    uint16_t values[SAMPLES];
    for (int i = 0; i < SAMPLES; i++) {
        values[i] = readADC(channel);
    }

    // uartPrintf("Raw: ");

    // for (int i = 0; i < SAMPLES; i++) {
    //     uartPrintf("%d ", values[i]);
    // }

    // uartPrintf("\r\nWhat: ");

    // if (channel == 0xD0) {
    //     for (int i = 0; i < SAMPLES; i++) {
    //         uartPrintf("%d ", (values[i] * XFAC) / 10000 + XOFFSET);
    //     }
    //     uartPrintf("\r\n");
    // }

    // Find min and max
    uint16_t min = values[0], max = values[0];
    for (int i = 1; i < SAMPLES; i++) {
        if (values[i] < min) min = values[i];
        if (values[i] > max) max = values[i];
    }

    // Calculate average excluding min and max
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
        if (values[i] != min && values[i] != max) {
            sum += values[i];
        }
    }

    return sum / (SAMPLES - 2);
}

uint16_t Touchscreen::readX() {
    return readChannel(0xD0);
}

uint16_t Touchscreen::readY() {
    return readChannel(0x90);
}

