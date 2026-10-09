#pragma once
#include <Arduino.h>
#include "common/HardwareProtocol.h"

template<class Output>
void printSensorHelp(Output& out) {
  out.println(F("TEST commands (finish with Enter / Newline):"));
  out.println(F("temp 0..2 | pressure 0..2 | voc 0..1"));
  out.println(F("humidity 0..2 | dust 0..1 | help"));
  out.println(F("9600 baud. Binary v2 requests are also accepted."));
}

// Только диагностика: расшифровываем уже сформированный ответ, не читаем
// датчик повторно и не меняем valid/fresh. HEX — точные 28 байтов с CRC.
template<class Output>
void printSensorReply(Output& out, const hardware_api::Frame& reply) {
  using namespace hardware_api;
  out.print(F("RX command=0x")); out.print(reply.command, HEX);
  out.print(F(" idx=")); out.print(reply.idx);
  out.print(F(" tx=")); out.print(reply.tx_id);
  out.print(F(" status=")); out.println(reply.status);
  if (reply.status == OK) {
    const uint32_t value = readU32(reply.data);
    switch (reply.command) {
      case kGetTemp:
        out.print(F("Temperature: ")); out.print(static_cast<int32_t>(value) / 100.0, 2);
        out.println(F(" C")); break;
      case kGetPressure:
        out.print(F("Pressure: ")); out.print(value); out.println(F(" Pa")); break;
      case kGetVoc:
        out.print(F("VOC Index: ")); out.println(value); break;
      case kGetHumidity:
        out.print(F("Humidity: ")); out.print(value / 100.0, 2); out.println(F(" % RH")); break;
      case kGetDust:
        out.print(F("Dust ADC: ")); out.print(readU16(reply.data));
        out.print(F("; voltage: ")); out.print(readU16(reply.data + 2) / 1000.0, 3);
        out.println(F(" V (not concentration)")); break;
      case kGetDoor:
        out.print(F("Door switch: ")); out.println(reply.data[0]); break;
    }
    if (reply.command != kGetDoor) {
      out.print(F("valid=")); out.print(reply.data[4] & 1);
      out.print(F(" fresh=")); out.print((reply.data[4] >> 1) & 1);
      out.print(F(" age_ms=")); out.println(readU16(reply.data + 5));
    }
  } else {
    out.println(F("No valid measurement; see protocol status code."));
  }
  uint8_t bytes[kFrameSize]; encode(reply, bytes);
  out.print(F("TX HEX: "));
  for (uint8_t i = 0; i < kFrameSize; ++i) {
    if (i) out.print(' ');
    if (bytes[i] < 16) out.print('0');
    out.print(bytes[i], HEX);
  }
  out.println();
}
