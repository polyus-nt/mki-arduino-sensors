#pragma once

#include <Arduino.h>

// Analog limit switch. A value below the threshold means the switch is active.
class Koltsevik {
 public:
  explicit Koltsevik(uint8_t pin, int threshold = 50)
      : pin_(pin), threshold_(threshold) {}

  void begin() const {
    pinMode(pin_, INPUT_PULLUP);
  }

  // Returns 1 for an active switch and 0 otherwise.
  int read() const {
    return analogRead(pin_) < threshold_ ? 1 : 0;
  }

  int rawValue() const {
    return analogRead(pin_);
  }

 private:
  const uint8_t pin_;
  const int threshold_;
};
