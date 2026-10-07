#pragma once
#include <Arduino.h>
#include "SensorApi.h"
#include "../BMP280.h"
#include "../VocSensor.h"

#ifdef HARDWARE_API_BOARD_HEADER
#include HARDWARE_API_BOARD_HEADER
#endif
#ifndef API_LINK_MISO
#define API_LINK_MISO 2
#define API_LINK_MOSI 3
#define API_LINK_CLK 4
#define API_LINK_CS 5
#define API_LINK_IRQ 6
#endif

// Door inputs are intentionally disabled until the physical mounting/pinout is known.
#ifndef API_DOOR0_PIN
#define API_DOOR0_PIN -1
#endif
#ifndef API_DOOR1_PIN
#define API_DOOR1_PIN -1
#endif
#ifndef API_DOOR0_CLOSED_LEVEL
#define API_DOOR0_CLOSED_LEVEL LOW
#endif
#ifndef API_DOOR1_CLOSED_LEVEL
#define API_DOOR1_CLOSED_LEVEL LOW
#endif

class BoardIO : public hardware_api::SensorHardware {
 public:
  BoardIO() : doorsReady_(false) {}
  void setup() {
    for (uint8_t i = 0; i < 3; ++i) bmp_[i].setup();
    for (uint8_t i = 0; i < 2; ++i) voc_[i].setup();
    // Check all pins before touching any: no overlap with the transport.
    const int pins[2] = {API_DOOR0_PIN, API_DOOR1_PIN};
    const int transport[5] = {API_LINK_MISO, API_LINK_MOSI, API_LINK_CLK,
                             API_LINK_CS, API_LINK_IRQ};
    if (pins[0] < 0 || pins[1] < 0 || pins[0] == pins[1]) return;
    if ((API_DOOR0_CLOSED_LEVEL != LOW && API_DOOR0_CLOSED_LEVEL != HIGH) ||
        (API_DOOR1_CLOSED_LEVEL != LOW && API_DOOR1_CLOSED_LEVEL != HIGH)) return;
    for (uint8_t i = 0; i < 2; ++i) {
      if (pins[i] >= NUM_DIGITAL_PINS) return;
      for (uint8_t j = 0; j < 5; ++j) if (pins[i] == transport[j]) return;
    }
    pinMode(pins[0], INPUT_PULLUP); pinMode(pins[1], INPUT_PULLUP);
    doorsReady_ = true;
  }
  hardware_api::Status temperature(uint8_t, hardware_api::Measurement&) override {
    return hardware_api::NOT_IMPLEMENTED;  // BMP280 remains a stub by agreement.
  }
  hardware_api::Status pressure(uint8_t, hardware_api::Measurement&) override {
    return hardware_api::NOT_IMPLEMENTED;
  }
  hardware_api::Status door(uint8_t idx, bool& closed) override {
    if (!doorsReady_) return hardware_api::NOT_IMPLEMENTED;
    if (idx == 0) closed = digitalRead(API_DOOR0_PIN) == API_DOOR0_CLOSED_LEVEL;
    else closed = digitalRead(API_DOOR1_PIN) == API_DOOR1_CLOSED_LEVEL;
    return hardware_api::OK;
  }
  hardware_api::Status unsupported(uint8_t, uint8_t) override {
    // VOC unknown; DHT11/GP2Y1010AU0F pins and successful wire layouts pending.
    return hardware_api::NOT_IMPLEMENTED;
  }
 private:
  BMP280 bmp_[3];
  VocSensor voc_[2];
  bool doorsReady_;
};
