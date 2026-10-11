#pragma once
#include "common/HardwareProtocol.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#define SENSOR_TEST_MATCH(text, literal) (strcmp_P((text), PSTR(literal)) == 0)
#else
#define SENSOR_TEST_MATCH(text, literal) (strcmp((text), (literal)) == 0)
#endif

// TEST-консоль без String/heap. Один ввод — либо строка, либо бинарный кадр.
// CR/LF внутри бинарного кадра не завершают текстовую команду.
class SensorTestInput {
 public:
  enum Event { NONE, REQUEST, HELP, ERROR };
  SensorTestInput() : size_(0), binaryLeft_(0), badLine_(false), lastMs_(0), firstMs_(0) {}
  void tick(uint32_t now) {
    if (binaryLeft_ && (static_cast<uint32_t>(now-lastMs_) >= 250UL ||
                       static_cast<uint32_t>(now-firstMs_) >= 1000UL)) binaryLeft_=0;
    if ((size_ || badLine_) && static_cast<uint32_t>(now-lastMs_) >= 5000UL) {
      size_=0; badLine_=false;
    }
  }
  Event feed(uint8_t byte, uint32_t now, hardware_api::Frame& request) {
    tick(now);
    lastMs_=now;
    if (binaryLeft_ || (!size_ && !badLine_ && byte==0xa5)) {
      if (!binaryLeft_) { binaryLeft_=hardware_api::kFrameSize; firstMs_=now; }
      line_[hardware_api::kFrameSize-binaryLeft_]=static_cast<char>(byte);
      --binaryLeft_;
      if (binaryLeft_) return NONE;
      return hardware_api::decode(reinterpret_cast<const uint8_t*>(line_),request) &&
          request.kind==hardware_api::kRequest ? REQUEST : ERROR;
    }
    if (byte=='\r' || byte=='\n') {
      if (badLine_) { size_=0; badLine_=false; return ERROR; }
      if (!size_) return NONE;
      line_[size_]='\0'; size_=0;
      return parse(request);
    }
    if (badLine_) return NONE;
    if ((byte<32 && byte!='\t') || byte>126 || size_>=sizeof(line_)-1) {
      badLine_=true; return NONE;
    }
    line_[size_++]=static_cast<char>(byte);
    return NONE;
  }
 private:
  static bool space(char c) { return c==' ' || c=='\t'; }
  Event parse(hardware_api::Frame& request) {
    using namespace hardware_api;
    char* name=line_; while (space(*name)) ++name;
    if (!*name) return NONE;
    char* arg=name; while (*arg && !space(*arg)) ++arg;
    if (*arg) *arg++='\0';
    while (space(*arg)) ++arg;
    if (SENSOR_TEST_MATCH(name,"help")) return *arg ? ERROR : HELP;
    uint8_t command=0, count=0;
    if (SENSOR_TEST_MATCH(name,"temp")) { command=kGetTemp; count=3; }
    else if (SENSOR_TEST_MATCH(name,"pressure")) { command=kGetPressure; count=3; }
    else if (SENSOR_TEST_MATCH(name,"voc")) { command=kGetVoc; count=2; }
    else if (SENSOR_TEST_MATCH(name,"humidity")) { command=kGetHumidity; count=3; }
    else if (SENSOR_TEST_MATCH(name,"dust")) { command=kGetDust; count=1; }
    else return ERROR;
    if (command == kGetDust ? *arg != '\0' : (*arg<'0' || *arg>'9')) return ERROR;
    uint16_t idx=0;
    while (*arg>='0' && *arg<='9') {
      idx=idx*10+static_cast<uint8_t>(*arg++-'0');
      if (idx>=count) return ERROR;
    }
    while (space(*arg)) ++arg;
    if (*arg) return ERROR;
    request=Frame{}; request.version=kVersion; request.kind=kRequest;
    request.node=kSensorsNode; request.command=command; request.idx=static_cast<uint8_t>(idx);
    // Локальная read-only диагностика, не очередь сервера: фиксированная пара.
    request.tx_id=1; request.session_id=1;
    return REQUEST;
  }
  // Общий буфер строки/кадра экономит RAM Uno; режимы не смешиваются.
  char line_[32];
  uint8_t size_, binaryLeft_;
  bool badLine_;
  uint32_t lastMs_, firstMs_;
};
#undef SENSOR_TEST_MATCH
