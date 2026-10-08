#pragma once
// DHT0 пересекается с IRQ Austere. Остальные группы должны остаться безопасны.
#include "compile_only_dht.h"
#undef API_DHT0_PIN
#define API_DHT0_PIN 6
