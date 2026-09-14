#include <assert.h>

#include "SensorProtocol.h"
#include "TemperatureReadingState.h"

int main() {
  using sensor_protocol::EncodedTemperature;
  using sensor_protocol::decodeTemperature;
  using sensor_protocol::encodeTemperature;

  EncodedTemperature positive = encodeTemperature(25.50f);
  assert(positive.whole == 0x19 && positive.fraction == 0x81);
  EncodedTemperature negative = encodeTemperature(-0.25f);
  assert(negative.whole == 0xFF && negative.fraction == 0xC1);
  assert(fabsf(decodeTemperature(negative) + 0.25f) < 0.011f);
  TemperatureReadingState reading;
  // An absent sensor and a CRC error both retain no valid/fresh data before
  // the first successful conversion.
  TemperatureReadingState::Value absent = reading.retained();
  assert(!absent.valid && !absent.fresh);
  TemperatureReadingState::Value crcError = reading.complete(false, 85.0f);
  assert(!crcError.valid && !crcError.fresh);
  TemperatureReadingState::Value good = reading.complete(true, 21.25f);
  assert(good.valid && good.fresh);
  crcError = reading.complete(false, 85.0f);
  assert(crcError.valid && !crcError.fresh && fabsf(crcError.value - 21.25f) < 0.001f);
  assert(sensor_protocol::sensorFlags(true, false, true, false) == 0x05);
  return 0;
}
