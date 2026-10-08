#pragma once
#include <Arduino.h>
// Один концевик двери. Полярность зависит от монтажа, задаётся в BoardConfig.
class DoorSwitch {
 public:
  DoorSwitch() : pin_(-1), closedLevel_(LOW) {}
  void begin(int pin, int closedLevel) {
    pin_ = pin; closedLevel_ = closedLevel; pinMode(pin_, INPUT_PULLUP);
  }
  bool closed() const { return digitalRead(pin_) == closedLevel_; }
 private:
  int pin_, closedLevel_;
};
