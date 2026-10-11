#pragma once
#include "common/HardwareProtocol.h"

namespace hardware_api {
struct Measurement {
  uint32_t value;  // signed centidegrees are encoded as their u32 bit pattern
  uint8_t flags;
  uint16_t ageMs;
};
class SensorHardware {
 public:
  virtual ~SensorHardware() {}
  virtual Status temperature(uint8_t idx, Measurement& result) = 0;
  virtual Status pressure(uint8_t idx, Measurement& result) = 0;
  virtual Status door(uint8_t idx, bool& closed) = 0;
  // Безразмерный VOC Index Sensirion 1..500: не ppm и не сырой SRAW.
  virtual Status voc(uint8_t, Measurement&) { return NOT_IMPLEMENTED; }
  // Влажность DHT11: unsigned u32 в сотых процента RH (0..10000).
  virtual Status humidity(uint8_t, Measurement&) { return NOT_IMPLEMENTED; }
  // Пыль: старшие 16 бит value — ADC, младшие — мВ, без концентрации.
  virtual Status dust(Measurement&) { return NOT_IMPLEMENTED; }
  virtual Status unsupported(uint8_t command, uint8_t idx) = 0;
};
class SensorApi {
 public:
  explicit SensorApi(SensorHardware& hardware) : hardware_(hardware) {}
  Frame handle(const Frame& request) {
    const Status checked = validateRequest(request, kSensorsNode);
    Frame reply = response(request, checked);
    if (checked != OK) return reply;
    Status status = NOT_IMPLEMENTED;
    if (request.command == kGetDoor) {
      bool closed = false;
      status = hardware_.door(request.idx, closed);
      if (status == OK) reply.data[0] = closed ? 1 : 0;
    } else if (request.command == kGetTemp || request.command == kGetPressure ||
               request.command == kGetVoc || request.command == kGetHumidity || request.command == kGetDust) {
      Measurement measurement = {};
      status = request.command == kGetTemp
          ? hardware_.temperature(request.idx, measurement)
          : request.command == kGetPressure ? hardware_.pressure(request.idx, measurement)
          : request.command == kGetVoc ? hardware_.voc(request.idx, measurement)
          : request.command == kGetHumidity ? hardware_.humidity(request.idx, measurement)
          : hardware_.dust(measurement);
      if (status == OK) {
        // Ноль blackout и raw/ppm не должны попасть в успешный ответ GET_VOC.
        if (request.command == kGetVoc && (measurement.value < 1 || measurement.value > 500))
          return response(request, INTERNAL_ERROR);
        // GET_HUMIDITY (0x24): idx=0..2, data запроса=0. Ответ — сотые % RH,
        // valid/fresh, u16 age_ms, 00. Например, 55.5% -> u32 5550.
        if (request.command == kGetHumidity && measurement.value > 10000)
          return response(request, INTERNAL_ERROR);
        if (request.command == kGetDust && (measurement.value >> 16) > 1023)
          return response(request, INTERNAL_ERROR);
        if ((measurement.flags != 1 && measurement.flags != 3) ||
            measurement.ageMs > 65534) return response(request, INTERNAL_ERROR);
        writeU32(reply.data, measurement.value);
        reply.data[4] = measurement.flags;
        writeU16(reply.data + 5, measurement.ageMs);
      }
    } else {
      status = hardware_.unsupported(request.command, request.idx);
      // There is intentionally no successful numeric layout for these yet.
      if (status == OK) status = NOT_IMPLEMENTED;
    }
    return status == OK ? reply : response(request, status);
  }
 private:
  SensorHardware& hardware_;
};
}  // namespace hardware_api
