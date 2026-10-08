#pragma once
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstdlib>
using word = uint16_t;
#define microsecondsToClockCycles(value) ((value) * 16UL)
#ifndef LOW
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
#endif
#define OUTPUT 1
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
void noInterrupts();
void interrupts();
using boolean = bool;
#define F_CPU 16000000UL
#define F(value) value
#define HEX 16
class Print {
 public:
  template<class T> void print(T, int = 10) {}
  template<class T> void println(T, int = 10) {}
  void println() {}
};
unsigned long millis();
void delay(unsigned long value);
void delayMicroseconds(unsigned int value);
uint8_t digitalPinToPort(uint8_t pin);
uint8_t digitalPinToBitMask(uint8_t pin);
volatile uint8_t* portOutputRegister(uint8_t port);
volatile uint8_t* portModeRegister(uint8_t port);
volatile uint8_t* portInputRegister(uint8_t port);
