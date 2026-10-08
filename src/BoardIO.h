#pragma once
#include <Arduino.h>
#include "../BoardConfig.h"
#include "SensorApi.h"
#include "../BMP280.h"
#include "../SGP40.h"
#include "../DoorSwitch.h"

#ifdef API_BMP_CONFIGURED
#include "BMP280SoftwareBus.h"
#endif
#ifdef API_VOC_CONFIGURED
#include "SGP40SoftwareBus.h"
#endif

class BoardIO : public hardware_api::SensorHardware {
 public:
  BoardIO() : doorsReady_(false), bmpReady_(false), vocReady_(false), nextSensor_(0)
#ifdef API_BMP_CONFIGURED
      , buses_{{API_BMP_SDA0_PIN, API_BMP_SCL_PIN},
               {API_BMP_SDA1_PIN, API_BMP_SCL_PIN},
               {API_BMP_SDA2_PIN, API_BMP_SCL_PIN}}
#endif
#ifdef API_VOC_CONFIGURED
      , vocBuses_{{API_VOC0_SDA_PIN, API_VOC0_SCL_PIN},
                  {API_VOC1_SDA_PIN, API_VOC1_SCL_PIN}}
#endif
  {}
  void setup() {
    doorsReady_ = bmpReady_ = vocReady_ = false; nextSensor_ = 0;
    for (uint8_t i = 0; i < 3; ++i) bmp_[i].setup(millis());
    for (uint8_t i = 0; i < 2; ++i) voc_[i].setup(millis());
    // Check all pins before touching any: no overlap with the transport.
    const int pins[2] = {API_DOOR0_PIN, API_DOOR1_PIN};
    const int transport[5] = {API_LINK_MISO, API_LINK_MOSI, API_LINK_CLK,
                             API_LINK_CS, API_LINK_IRQ};
#ifdef API_VOC_CONFIGURED
    const int vocPins[4] = {API_VOC0_SDA_PIN, API_VOC0_SCL_PIN,
                           API_VOC1_SDA_PIN, API_VOC1_SCL_PIN};
#endif
#ifdef API_BMP_CONFIGURED
    const int bmpPins[4] = {API_BMP_SCL_PIN, API_BMP_SDA0_PIN,
                           API_BMP_SDA1_PIN, API_BMP_SDA2_PIN};
    bool validBmp = true;
    for (uint8_t i = 0; i < 4; ++i) {
      if (bmpPins[i] < 0 || bmpPins[i] >= NUM_DIGITAL_PINS) validBmp = false;
      for (uint8_t j = 0; j < i; ++j) if (bmpPins[i] == bmpPins[j]) validBmp = false;
      for (uint8_t j = 0; j < 5; ++j) if (bmpPins[i] == transport[j]) validBmp = false;
      for (uint8_t j = 0; j < 2; ++j) if (bmpPins[i] == pins[j]) validBmp = false;
#ifdef API_VOC_CONFIGURED
      for (uint8_t j = 0; j < 4; ++j) if (bmpPins[i] == vocPins[j]) validBmp = false;
#endif
    }
    if (validBmp) {
      for (uint8_t i = 0; i < 3; ++i) {
        buses_[i].begin(); bmp_[i].attach(buses_[i], millis());
      }
      bmpReady_ = true;
    }
#endif
#ifdef API_VOC_CONFIGURED
    bool validVoc = true;
    for (uint8_t i = 0; i < 4; ++i) {
      if (vocPins[i] < 0 || vocPins[i] >= NUM_DIGITAL_PINS) validVoc = false;
      for (uint8_t j = 0; j < i; ++j) if (vocPins[i] == vocPins[j]) validVoc = false;
      for (uint8_t j = 0; j < 5; ++j) if (vocPins[i] == transport[j]) validVoc = false;
      for (uint8_t j = 0; j < 2; ++j) if (vocPins[i] == pins[j]) validVoc = false;
#ifdef API_BMP_CONFIGURED
      for (uint8_t j = 0; j < 4; ++j) if (vocPins[i] == bmpPins[j]) validVoc = false;
#endif
    }
    if (validVoc) {
      for (uint8_t i = 0; i < 2; ++i) {
        vocBuses_[i].begin(); voc_[i].attach(vocBuses_[i], millis());
      }
      vocReady_ = true;
    }
#endif
    if (pins[0] < 0 || pins[1] < 0 || pins[0] == pins[1]) return;
    if ((API_DOOR0_CLOSED_LEVEL != LOW && API_DOOR0_CLOSED_LEVEL != HIGH) ||
        (API_DOOR1_CLOSED_LEVEL != LOW && API_DOOR1_CLOSED_LEVEL != HIGH)) return;
    for (uint8_t i = 0; i < 2; ++i) {
      if (pins[i] >= NUM_DIGITAL_PINS) return;
      for (uint8_t j = 0; j < 5; ++j) if (pins[i] == transport[j]) return;
#ifdef API_BMP_CONFIGURED
      for (uint8_t j = 0; j < 4; ++j) if (pins[i] == bmpPins[j]) return;
#endif
#ifdef API_VOC_CONFIGURED
      for (uint8_t j = 0; j < 4; ++j) if (pins[i] == vocPins[j]) return;
#endif
    }
    doors_[0].begin(pins[0], API_DOOR0_CLOSED_LEVEL);
    doors_[1].begin(pins[1], API_DOOR1_CLOSED_LEVEL);
    doorsReady_ = true;
  }
  void poll(uint32_t now) {
    // Один из пяти датчиков за idle-итерацию, не две I²C-операции подряд.
    if (nextSensor_ < 3) {
      if (bmpReady_) bmp_[nextSensor_].poll(now);
    } else if (vocReady_) voc_[nextSensor_ - 3].poll(now);
    nextSensor_ = static_cast<uint8_t>((nextSensor_ + 1) % 5);
  }
  hardware_api::Status temperature(uint8_t idx, hardware_api::Measurement& result) override {
    if (idx >= 3) return hardware_api::INVALID_INDEX;
    if (!bmpReady_) return hardware_api::NOT_IMPLEMENTED;
    const BMP280::Reading reading = bmp_[idx].readTemperature(millis());
    return encodeReading(reading, true, result);
  }
  hardware_api::Status pressure(uint8_t idx, hardware_api::Measurement& result) override {
    if (idx >= 3) return hardware_api::INVALID_INDEX;
    if (!bmpReady_) return hardware_api::NOT_IMPLEMENTED;
    const BMP280::Reading reading = bmp_[idx].read(millis());
    return encodeReading(reading, false, result);
  }
  hardware_api::Status door(uint8_t idx, bool& closed) override {
    if (idx >= 2) return hardware_api::INVALID_INDEX;
    if (!doorsReady_) return hardware_api::NOT_IMPLEMENTED;
    closed = doors_[idx].closed();
    return hardware_api::OK;
  }
  hardware_api::Status unsupported(uint8_t, uint8_t) override {
    // DHT11/GP2Y1010AU0F pins and successful wire layouts pending.
    return hardware_api::NOT_IMPLEMENTED;
  }
  hardware_api::Status voc(uint8_t idx, hardware_api::Measurement& result) override {
    if (idx >= 2) return hardware_api::INVALID_INDEX;
    if (!vocReady_) return hardware_api::NOT_IMPLEMENTED;
    const SGP40::Reading reading = voc_[idx].read(millis());
    if (!reading.valid) return hardware_api::SENSOR_UNAVAILABLE;
    result.value = reading.value; // u32 big-endian, VOC Index 1..500 без масштаба.
    result.flags = reading.fresh ? 3 : 1; result.ageMs = reading.ageMs;
    return hardware_api::OK;
  }
 private:
  static hardware_api::Status encodeReading(const BMP280::Reading& reading, bool temperature,
                                           hardware_api::Measurement& result) {
    if (!reading.valid) return hardware_api::SENSOR_UNAVAILABLE;
    const double rounded = round(static_cast<double>(reading.value) * (temperature ? 100.0 : 1.0));
    if (!isfinite(rounded)) return hardware_api::SENSOR_UNAVAILABLE;
    if (temperature) {
      if (rounded <= -2147483648.0 || rounded >= 2147483647.0) return hardware_api::SENSOR_UNAVAILABLE;
      result.value = static_cast<uint32_t>(static_cast<int32_t>(rounded));
    } else {
      if (rounded < 0 || rounded >= 4294967295.0) return hardware_api::SENSOR_UNAVAILABLE;
      result.value = static_cast<uint32_t>(rounded);
    }
    result.flags = reading.fresh ? 3 : 1;
    result.ageMs = reading.ageMs;
    return hardware_api::OK;
  }
  BMP280 bmp_[3];
  SGP40 voc_[2];
  DoorSwitch doors_[2];
  bool doorsReady_, bmpReady_, vocReady_;
  uint8_t nextSensor_;
#ifdef API_BMP_CONFIGURED
  BMP280SoftwareBus buses_[3];
#endif
#ifdef API_VOC_CONFIGURED
  SGP40SoftwareBus vocBuses_[2];
#endif
};
