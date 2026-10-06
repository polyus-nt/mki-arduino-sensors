#pragma once

#include <math.h>

// Заглушка датчика давления BMP280: без доступа к аппаратной шине.
// Каждый экземпляр представляет один датчик; единицы и драйвер уточним позже.
class BMP280 {
 public:
  struct Reading {
    float value;
    bool valid;
    bool fresh;
  };

  void setup() {}

  Reading read() const { return {NAN, false, false}; }

  bool isConnected() const { return false; }
};
