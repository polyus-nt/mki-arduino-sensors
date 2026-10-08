#include <cassert>
#include <cstdio>
#include "../src/DHT11Hardware.h"
static uint32_t timeNow = 0;
static unsigned reads = 0, starts = 0, locks = 0;
static bool capturing = false, corrupt = false, stuck = false;
static unsigned pulse = 0, remaining = 0;
static unsigned duration(unsigned index) {
  if (index < 2) return 80;
  if (index >= 82) return 50;
  if ((index & 1) == 0) return 50;
  const uint8_t data[] = {55, 5, 24, 0, static_cast<uint8_t>(84 ^ corrupt)};
  const unsigned bit = (index - 3) / 2;
  return (data[bit / 8] & (1U << (7 - bit % 8))) ? 70 : 28;
}
unsigned long millis() { return timeNow; }
void delay(unsigned long value) { timeNow += value; }
void delayMicroseconds(unsigned int) {
  if (!capturing) { capturing = true; pulse = 0; remaining = duration(0); }
}
void pinMode(int, int mode) { if (mode == OUTPUT) { ++starts; capturing = false; } }
void digitalWrite(int, int) {}
int digitalRead(int) {
  ++reads;
  assert(capturing);
  if (stuck && pulse >= 5) return HIGH;
  if (remaining == 0) { ++pulse; remaining = duration(pulse); }
  --remaining; return pulse & 1 ? HIGH : LOW;
}
void noInterrupts() { ++locks; }
void interrupts() { assert(locks); --locks; }
int main() {
  DHT11Hardware reader(13); reader.begin();
  float humidity = NAN;
  assert(reader.humidity(humidity) && humidity == 55.5f && locks == 0);
  assert(timeNow == 21 && starts == 1);
  assert(reader.humidity(humidity) && starts == 2); // force: новый сигнал, не кэш
  corrupt = true; assert(!reader.humidity(humidity) && isnan(humidity) && locks == 0);
  corrupt = false; stuck = true; reads = 0;
  assert(!reader.humidity(humidity) && locks == 0);
  assert(reads < 20000); // первый timeout обрывает захват, а не ещё 75 импульсов
  std::puts("DHT GPIO: real Adafruit pulse decoding, checksum, forced read, early timeout passed");
}
