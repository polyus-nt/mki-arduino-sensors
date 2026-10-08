#pragma once
#include <Arduino.h>
#include "../GP2Y1010AU0F.h"
// Пины передаёт BoardConfig. LED активен LOW, в покое HIGH.
class GP2Y1010Hardware : public GP2Y1010AU0F::Reader {
 public:
  GP2Y1010Hardware(uint8_t led, uint8_t analog) : led_(led), analog_(analog) {}
  void begin() override {
    digitalWrite(led_, HIGH);
    pinMode(led_, OUTPUT); pinMode(analog_, INPUT);
  }
  bool sample(uint16_t& adc) override {
    digitalWrite(led_, LOW); delayMicroseconds(280);
    const int raw = analogRead(analog_);
    delayMicroseconds(40); digitalWrite(led_, HIGH);
    if (raw < 0 || raw > 1023) return false;
    adc = static_cast<uint16_t>(raw); return true;
  }
  uint32_t completedAt(uint32_t) const override { return millis(); }
 private:
  uint8_t led_, analog_;
};
