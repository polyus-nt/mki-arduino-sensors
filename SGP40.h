#pragma once
#include <stdint.h>
extern "C" {
#include "src/sensirion/sensirion_gas_index_algorithm.h"
}
// Один SGP40, собственная шина, алгоритм и кэш; без динамической памяти.
class SGP40 {
 public:
  class Bus {
   public:
    virtual ~Bus() {}
    virtual bool send(const uint8_t*, uint8_t) = 0;
    virtual bool receive(uint8_t*, uint8_t) = 0;
    // Аппаратный адаптер возвращает время ПОСЛЕ синхронного I²C, mock — now.
    virtual uint32_t completedAt(uint32_t now) const { return now; }
  };
  struct Reading { uint16_t value; bool valid; bool fresh; uint16_t ageMs; };
  explicit SGP40(Bus* bus = nullptr) : bus_(bus) { setup(); }
  void attach(Bus& bus, uint32_t now = 0) { bus_ = &bus; setup(now); }
  void setup(uint32_t now = 0) {
    GasIndexAlgorithm_init(&algorithm_, GasIndexAlgorithm_ALGORITHM_TYPE_VOC);
    waiting_ = connected_ = valid_ = fresh_ = false;
    lastStart_ = now - 1000UL; startedAt_ = measuredAt_ = now; value_ = 0;
  }
  bool isConnected() const { return connected_; }
  static uint8_t crc(const uint8_t* data, uint8_t count) {
    uint8_t value = 0xff;
    for (uint8_t i = 0; i < count; ++i) {
      value ^= data[i];
      for (uint8_t bit = 0; bit < 8; ++bit)
        value = static_cast<uint8_t>((value << 1) ^ ((value & 0x80) ? 0x31 : 0));
    }
    return value;
  }
  // Только отправка ИЛИ чтение за вызов; ожидание 35 мс не блокирует loop.
  bool poll(uint32_t now) {
    if (!bus_) return false;
    if (waiting_) {
      const uint32_t elapsed = static_cast<uint32_t>(now - startedAt_);
      if (elapsed < 35UL) return false;
      waiting_ = false;
      if (elapsed >= 1000UL) return fail();
      uint8_t bytes[3];
      if (!bus_->receive(bytes, 3) || crc(bytes, 2) != bytes[2]) return fail();
      const uint16_t raw = (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
      // Официальный core игнорирует raw >=65000: не выдаём старый index как fresh.
      if (raw == 0 || raw >= 65000) return fail();
      connected_ = true;
      int32_t index = 0;
      GasIndexAlgorithm_process(&algorithm_, raw, &index);
      // Стартовый blackout (~45 с) даёт 0, но это не валидный VOC Index.
      if (index < 1 || index > 500) return false;
      value_ = static_cast<uint16_t>(index); measuredAt_ = bus_->completedAt(now);
      valid_ = fresh_ = true; return true;
    }
    const uint32_t elapsed = static_cast<uint32_t>(now - lastStart_);
    if (elapsed < 1000UL) return false;
    // Пропуск целого дополнительного интервала: reset, без нагоняющего burst.
    if (elapsed >= 2000UL) fail();
    lastStart_ = startedAt_ = now;
    // Фиксированная компенсация из примера RH=50%, T=25°C.
    const uint8_t command[] = {0x26, 0x0f, 0x80, 0x00, 0xa2, 0x66, 0x66, 0x93};
    if (!bus_->send(command, sizeof(command))) return fail();
    // 35 мс после конца команды, а не начала потенциально 20-мс I²C-операции.
    startedAt_ = bus_->completedAt(now);
    waiting_ = true; return false;
  }
  Reading read(uint32_t now) {
    const uint32_t age = static_cast<uint32_t>(now - measuredAt_);
    Reading result = {value_, valid_, fresh_, static_cast<uint16_t>(age > 65534UL ? 65534 : age)};
    fresh_ = false; return result;
  }
 private:
  bool fail() {
    connected_ = fresh_ = false;
    GasIndexAlgorithm_reset(&algorithm_);
    return false;
  }
  Bus* bus_;
  GasIndexAlgorithmParams algorithm_;
  uint32_t lastStart_, startedAt_, measuredAt_;
  uint16_t value_;
  bool waiting_, connected_, valid_, fresh_;
};
