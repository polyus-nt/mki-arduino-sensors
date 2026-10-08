#include <cassert>
#include <cstdio>
#include "../src/softwarewire/SoftwareWire.h"

// Simulated GPIO: SDA pin=0, SCL pin=1. Delays expose SDA ACK (low)
// after the START; SCL is then held low to emulate a stuck slave.
static volatile uint8_t outputReg = 0, directionReg = 0, inputReg = 3;
static uint32_t now = 0;
static bool stuck = false;
unsigned long millis() { const uint32_t result = now; ++now; return result; }
void delay(unsigned long value) { now += value; }
void delayMicroseconds(unsigned int) {
  if (directionReg & 1) { inputReg = stuck ? 0 : 2; }
}
uint8_t digitalPinToPort(uint8_t) { return 0; }
uint8_t digitalPinToBitMask(uint8_t pin) { return static_cast<uint8_t>(1U << pin); }
volatile uint8_t* portOutputRegister(uint8_t) { return &outputReg; }
volatile uint8_t* portModeRegister(uint8_t) { return &directionReg; }
volatile uint8_t* portInputRegister(uint8_t) { return &inputReg; }

static void checkTimeout(uint32_t start) {
  SoftwareWire wire(0, 1, false);
  wire.begin(); wire.setClock(50000); wire.setTimeout(20);
  inputReg = 3; now = start; stuck = true;
  wire.beginTransmission(uint8_t(0x76));
  wire.write(uint8_t(0x88));
  assert(wire.endTransmission(false) == SOFTWAREWIRE_OTHER);
  assert(wire.getTimeoutFlag());
  // A read in the same transaction must not restart a fresh per-bit timeout
  // or publish bytes despite a failed clock stretch.
  assert(wire.requestFrom(uint8_t(0x76), uint8_t(24)) == 0);
  assert(wire.available() == 0 && wire.read() == -1);
  assert(static_cast<uint32_t>(now - start) < 30);
  assert((directionReg & 3) == 0); // both lines released by STOP
  // Next transaction resets the latched error and successfully addresses the slave.
  stuck = false; inputReg = 3;
  wire.beginTransmission(uint8_t(0x76));
  assert(wire.endTransmission() == SOFTWAREWIRE_NO_ERROR);
  assert(!wire.getTimeoutFlag());
}

int main() {
  checkTimeout(100);
  checkTimeout(UINT32_MAX - 10);
  {
    // Oversized reads must not leave a previous successful result in RX.
    SoftwareWire wire(0, 1, false);
    wire.begin(); wire.setClock(50000); wire.setTimeout(100);
    now = 0; inputReg = 3; stuck = false;
    assert(wire.requestFrom(uint8_t(0x76), uint8_t(1)) == 1);
    assert(wire.available() == 1);
    assert(wire.requestFrom(uint8_t(0x76), uint8_t(33)) == 0);
    assert(wire.available() == 0 && wire.read() == -1);
  }
  {
    // Regression for upstream's silent success: a read starts on a free bus,
    // then the first clock stretch hangs. No fabricated calibration bytes.
    SoftwareWire wire(0, 1, false);
    wire.begin(); wire.setClock(50000); wire.setTimeout(20);
    now = 0; inputReg = 3; stuck = true;
    assert(wire.requestFrom(uint8_t(0x76), uint8_t(24)) == 0);
    assert(wire.getTimeoutFlag());
    assert(wire.available() == 0 && now < 30);
    assert((directionReg & 3) == 0);
  }
  std::puts("SoftwareWire: whole-operation timeout, error propagation, released pins, wrap passed");
}
