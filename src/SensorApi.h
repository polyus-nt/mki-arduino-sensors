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
  // Units for VOC/humidity/dust must be settled before returning numeric success.
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
    } else if (request.command == kGetTemp || request.command == kGetPressure) {
      Measurement measurement = {};
      status = request.command == kGetTemp
          ? hardware_.temperature(request.idx, measurement)
          : hardware_.pressure(request.idx, measurement);
      if (status == OK) {
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
