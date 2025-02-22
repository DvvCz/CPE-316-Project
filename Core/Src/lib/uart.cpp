#include "uart.hpp"

#include <cstdarg>
#include <cstdbool>
#include <cstdio>
#include <cstring>

#include "main.h"

/* Returns true if successfully read. */
bool uartRead(char* buf, const size_t len) {
  return HAL_UART_Receive(&huart2, (uint8_t*)buf, len, HAL_MAX_DELAY) == HAL_OK;
}

/* Returns character read, or -1 for error. */
char uartReadChar() {
  char buf[1];

  if (!uartRead(buf, sizeof(buf))) {
    return -1;
  }

  return buf[0];
}

bool uartPrintf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  bool status = HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY) == HAL_OK;

  va_end(args);

  return status;
}