#pragma once
#include "softwarewire/SoftwareWire.h"
#include "../SGP40.h"
// Две отдельные шины: одинаковый адрес SGP40 0x59, внутренние подтяжки выключены.
class SGP40SoftwareBus : public SGP40::Bus {
 public:
  SGP40SoftwareBus(uint8_t sda, uint8_t scl) : wire_(sda, scl, false) {}
  void begin() { wire_.begin(); wire_.setClock(50000); wire_.setTimeout(20); }
  uint32_t completedAt(uint32_t) const override { return millis(); }
  bool send(const uint8_t* bytes, uint8_t count) override {
    wire_.beginTransmission(0x59);
    bool written = true;
    for (uint8_t i = 0; i < count; ++i) if (wire_.write(bytes[i]) != 1) written = false;
    const bool stopped = wire_.endTransmission() == 0;
    return written && stopped && !wire_.getTimeoutFlag();
  }
  bool receive(uint8_t* bytes, uint8_t count) override {
    if (wire_.requestFrom(static_cast<uint8_t>(0x59), count) != count || wire_.getTimeoutFlag()) return false;
    for (uint8_t i = 0; i < count; ++i) {
      const int value = wire_.read();
      if (value < 0) return false;
      bytes[i] = static_cast<uint8_t>(value);
    }
    return true;
  }
 private:
  SoftwareWire wire_;
};
