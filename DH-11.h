#pragma once

#include <Arduino.h>
#include <DHT.h>

// Обёртка для датчика температуры и влажности DHT11.
// Пример:
//   DH11 dht11(4);
//   dht11.setup();
//   float temperature = dht11.readTemperature();
class DH11 {
 public:
  explicit DH11(uint8_t pin)
      : dht_(pin, DHT11), lastTemperature_(-1.0f), lastReadMs_(0), hasRead_(false) {}

  // Инициализация датчика. Вызывать один раз из setup() скетча.
  void setup() {
    dht_.begin();
    hasRead_ = false;
  }

  // Температура в градусах Цельсия.
  // Датчик опрашивается не чаще одного раза в секунду; между опросами
  // возвращается последнее значение. При ошибке чтения возвращается -1.0f.
  float readTemperature() {
    const unsigned long now = millis();
    if (hasRead_ && now - lastReadMs_ < READ_INTERVAL_MS) {
      return lastTemperature_;
    }

    const float temperature = dht_.readTemperature();
    lastTemperature_ = isnan(temperature) ? -1.0f : temperature;
    lastReadMs_ = now;
    hasRead_ = true;
    return lastTemperature_;
  }

 private:
  static const unsigned long READ_INTERVAL_MS = 1000UL;

  DHT dht_;
  float lastTemperature_;
  unsigned long lastReadMs_;
  bool hasRead_;
};
