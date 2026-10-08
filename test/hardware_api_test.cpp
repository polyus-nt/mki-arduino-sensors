#include <assert.h>
#include <stdio.h>
#include "../src/SensorApi.h"
using namespace hardware_api;
struct Hardware : SensorHardware {
  unsigned reads = 0;
  uint8_t flags = 3; uint16_t age = 5000;
  Status measurementStatus = OK;
  Status temperature(uint8_t idx, Measurement& result) override {
    ++reads; result = {static_cast<uint32_t>(static_cast<int32_t>(-25 - idx)), flags, age};
    return measurementStatus;
  }
  Status pressure(uint8_t idx, Measurement& result) override {
    ++reads; result = {101325U + idx, flags, age}; return measurementStatus;
  }
  Status door(uint8_t idx, bool& closed) override {
    ++reads; closed = idx == 0; return OK;
  }
  Status unsupported(uint8_t, uint8_t) override { ++reads; return NOT_IMPLEMENTED; }
};
static Frame request(uint8_t command, uint8_t idx) {
  Frame frame = {}; frame.version = 1; frame.kind = 1; frame.tx_id = 100;
  frame.node = 13; frame.command = command; frame.idx = idx; return frame;
}
int main() {
  Hardware hardware; SensorApi api(hardware);
  for (uint8_t idx = 0; idx < 3; ++idx) {
    Frame reply = api.handle(request(kGetTemp, idx));
    assert(reply.status == OK && sameRequest(request(kGetTemp, idx), reply));
    assert(readU32(reply.data) == static_cast<uint32_t>(static_cast<int32_t>(-25 - idx)));
    assert(reply.data[4] == 3 && readU16(reply.data + 5) == 5000 && reply.data[7] == 0);
    reply = api.handle(request(kGetPressure, idx));
    assert(reply.status == OK && readU32(reply.data) == 101325U + idx);
    assert(api.handle(request(kGetHumidity, idx)).status == NOT_IMPLEMENTED);
  }
  assert(api.handle(request(kGetDoor, 0)).data[0] == 1);
  assert(api.handle(request(kGetDoor, 1)).data[0] == 0);
  assert(api.handle(request(kGetVoc, 1)).status == NOT_IMPLEMENTED);
  assert(api.handle(request(kGetDust, 1)).status == NOT_IMPLEMENTED);
  unsigned reads = hardware.reads;
  assert(api.handle(request(kGetTemp, 3)).status == INVALID_INDEX);
  assert(api.handle(request(kGetHumidity, 3)).status == INVALID_INDEX);
  assert(api.handle(request(kGetDoor, 2)).status == INVALID_INDEX);
  assert(api.handle(request(kGetDust, 2)).status == INVALID_INDEX);
  Frame invalid = request(kGetTemp, 0); invalid.data[0] = 1;
  assert(api.handle(invalid).status == INVALID_ARGUMENT);
  assert(api.handle(request(kSetDoorLock, 0)).status == UNKNOWN_COMMAND);
  assert(hardware.reads == reads);
  hardware.measurementStatus = NOT_IMPLEMENTED;
  Frame reply = api.handle(request(kGetTemp, 0));
  assert(reply.status == NOT_IMPLEMENTED && zeros(reply.data, 8));
  hardware.measurementStatus = SENSOR_UNAVAILABLE;
  assert(api.handle(request(kGetTemp, 0)).status == SENSOR_UNAVAILABLE);
  hardware.measurementStatus = OK; hardware.flags = 2;
  assert(api.handle(request(kGetTemp, 0)).status == INTERNAL_ERROR);
  hardware.flags = 1; hardware.age = 65534;
  assert(api.handle(request(kGetTemp, 0)).status == OK);
  hardware.age = 65535;
  assert(api.handle(request(kGetTemp, 0)).status == INTERNAL_ERROR);
  puts("sensors: three BMP280 slots, errors, quality, stub slots, doors passed");
}
