#pragma once
// Негативная проверка GPIO: VOC0 SCL пересекается с Austere IRQ=D6.
// Только для тестов отказа включения каналов, не прошивать.
#include "compile_only_sensors.h"
#undef API_VOC0_SCL_PIN
#define API_VOC0_SCL_PIN 6
