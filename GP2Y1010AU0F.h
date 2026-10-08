#pragma once
#include <math.h>

// Отдельный класс одного GP2Y1010AU0F. Драйвер импульса светодиода/АЦП и
// преобразование в концентрацию пыли ещё не реализованы: нужна карта пинов
// и согласованные калибровка/единицы. Заглушка не трогает аппаратуру.
class GP2Y1010AU0F {
 public:
  struct Reading { float value; bool valid; bool fresh; };
  void setup() {}
  Reading read() const { return {NAN, false, false}; }
  bool isConnected() const { return false; }
};
