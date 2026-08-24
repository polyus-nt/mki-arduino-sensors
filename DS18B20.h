#pragma once

#include <Arduino.h>
#include <OneWire.h>

// Датчик температуры Dallas DS18B20.
class DS18B20 {
 public:
  explicit DS18B20(uint8_t pin)
      : wire_(pin),
        lastTemperature_(0.0f),
        conversionStartedMs_(0),
        found_(false),
        conversionStarted_(false) {}

  // Ищет подключённый DS18B20. Вызывать один раз из setup() скетча.
  void setup() {
    found_ = findSensor();
    if (found_) {
      startConversion();
    }
  }

  // Возвращает температуру в градусах Цельсия.
  // При ошибке возвращает последнее валидное значение (или 0.0f,
  // если корректных измерений ещё не было).
  // Измерение занимает до 750 мс, но этот метод не блокирует скетч:
  // пока измерение не готово, возвращается последнее значение.
  float readTemperature() {
    if (!found_) {
      if (!findSensor()) {
        return lastTemperature_;
      }
      startConversion();
      return lastTemperature_;
    }

    if (!conversionStarted_) {
      startConversion();
      return lastTemperature_;
    }

    if (millis() - conversionStartedMs_ < CONVERSION_TIME_MS) {
      return lastTemperature_;
    }

    float temperature;
    const bool readSucceeded = readCompletedTemperature(temperature);
    startConversion();

    if (!readSucceeded) {
      return lastTemperature_;
    }

    lastTemperature_ = temperature;
    return lastTemperature_;
  }

  // Checks whether a DS18B20 is currently visible on the OneWire bus.
  // Intended for diagnostics; normal measurements use readTemperature().
  bool isConnected() {
    found_ = findSensor();
    return found_;
  }

 private:
  static const unsigned long CONVERSION_TIME_MS = 750UL;

  void startConversion() {
    if (!wire_.reset()) {
      found_ = false;
      conversionStarted_ = false;
      return;
    }
    wire_.select(address_);
    wire_.write(0x44, 1);  // Запуск измерения, питание по линии данных.
    conversionStartedMs_ = millis();
    conversionStarted_ = true;
  }

  bool readCompletedTemperature(float& temperature) {
    if (!wire_.reset()) {
      found_ = false;
      conversionStarted_ = false;
      return false;
    }
    wire_.select(address_);
    wire_.write(0xBE);  // Чтение scratchpad.

    byte data[9];
    for (uint8_t i = 0; i < sizeof(data); ++i) {
      data[i] = wire_.read();
    }

    if (OneWire::crc8(data, 8) != data[8]) {
      return false;
    }

    const int16_t raw = (static_cast<int16_t>(data[1]) << 8) | data[0];
    temperature = static_cast<float>(raw) / 16.0f;
    return true;
  }

  bool findSensor() {
    wire_.reset_search();
    while (wire_.search(address_)) {
      if (OneWire::crc8(address_, 7) == address_[7] && address_[0] == 0x28) {
        return true;
      }
    }

    wire_.reset_search();
    return false;
  }

  OneWire wire_;
  byte address_[8];
  float lastTemperature_;
  unsigned long conversionStartedMs_;
  bool found_;
  bool conversionStarted_;
};
