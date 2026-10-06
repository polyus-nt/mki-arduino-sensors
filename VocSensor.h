#pragma once

#include <math.h>

// Заглушка VOC-датчика: модель и единицы измерения ещё не выбраны.
// Каждый экземпляр представляет один датчик; аппаратный драйвер добавим позже.
class VocSensor {
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
