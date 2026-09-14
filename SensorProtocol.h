#pragma once

#include <math.h>
#include <stdint.h>

namespace sensor_protocol {

struct EncodedTemperature {
  uint8_t whole;
  uint8_t fraction;
};

// The positional temperature format uses floor for the signed whole part, so
// -0.25 is represented as [-1, 191] and round-trips without a sign bit.
inline EncodedTemperature encodeTemperature(float value) {
  if (!isfinite(value)) value = 0.0f;
  if (value < -128.0f) value = -128.0f;
  if (value > 127.99f) value = 127.99f;
  int16_t whole = static_cast<int16_t>(floorf(value));
  uint16_t hundredths = static_cast<uint16_t>(lroundf((value - whole) * 100.0f));
  if (hundredths == 100) {
    ++whole;
    hundredths = 0;
  }
  EncodedTemperature result;
  result.whole = static_cast<uint8_t>(static_cast<int8_t>(whole));
  result.fraction = static_cast<uint8_t>(lroundf(hundredths * 255.0f / 99.0f));
  return result;
}

inline float decodeTemperature(EncodedTemperature encoded) {
  const int8_t whole = static_cast<int8_t>(encoded.whole);
  const uint8_t hundredths =
      static_cast<uint8_t>(lroundf(encoded.fraction * 99.0f / 255.0f));
  return whole + hundredths / 100.0f;
}

inline uint8_t sensorFlags(bool insideValid, bool insideFresh,
                           bool outsideValid, bool outsideFresh) {
  return (insideValid ? 0x01 : 0) | (insideFresh ? 0x02 : 0) |
         (outsideValid ? 0x04 : 0) | (outsideFresh ? 0x08 : 0);
}

}  // namespace sensor_protocol
