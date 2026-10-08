#pragma once
#include <math.h>

// Отдельный класс одного DHT11. Чтение влажности ещё не реализовано:
// распиновка и формат успешного ответа API не согласованы.
// Заглушка не обращается к GPIO и не выдаёт вымышленное измерение.
class DHT11 {
 public:
  struct Reading { float value; bool valid; bool fresh; };
  void setup() {}
  Reading read() const { return {NAN, false, false}; }
  bool isConnected() const { return false; }
};
