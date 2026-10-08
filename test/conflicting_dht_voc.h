#pragma once
// DHT0 и VOC0 SCL пытаются использовать один пин: обе группы выключаются.
#include "compile_only_dht.h"
#undef API_DHT0_PIN
#define API_DHT0_PIN 8
