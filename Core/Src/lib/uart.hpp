#ifndef UART_H
#define UART_H

#include <cstdbool>
#include <cstdlib>

[[nodiscard]] bool uartRead(char* buf, const size_t len);
[[nodiscard]] char uartReadChar();

bool uartPrintf(const char* fmt, ...);

#endif