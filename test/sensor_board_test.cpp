#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
#define NUM_DIGITAL_PINS 20
static uint32_t timeNow=0;
unsigned long millis() {return timeNow;}
static unsigned gpioCalls=0;
void pinMode(int, int) {++gpioCalls;}
int digitalRead(int pin) {return pin == 11 ? LOW : HIGH;}
// Подменяем только электрический транспорт: production BoardIO и драйверы настоящие.
#define SoftwareWire_h
static unsigned busBegins=0, operations=0;
class SoftwareWire {
 public:
  SoftwareWire(uint8_t, uint8_t, bool) : address_(0), position_(0) {}
  void begin() {++busBegins;}
  void setClock(uint32_t) {}
  void setTimeout(uint32_t) {}
  void beginTransmission(uint8_t address) {address_=address; ++operations;}
  unsigned write(uint8_t) {return 1;}
  uint8_t endTransmission(bool = true) {return 0;}
  bool getTimeoutFlag() const {return false;}
  uint8_t requestFrom(uint8_t address, uint8_t count) {
    address_=address; position_=0; if(address==0x59) ++operations; return count;
  }
  int read() {
    const uint8_t response[] = {0x61,0xa8,0xd1}; // raw25000 + correct CRC replaced below
    if(address_==0x59) return position_ < 2 ? response[position_++] : vocCrc_;
    return 0; // BMP chip-id unavailable; no fabricated pressure.
  }
  static uint8_t vocCrc_;
 private:
  uint8_t address_, position_;
};
uint8_t SoftwareWire::vocCrc_=0;
#include "../src/BoardIO.h"
#ifndef TEST_EXPECT_BUS_COUNT
#define TEST_EXPECT_BUS_COUNT 0
#endif
int main() {
  const uint8_t raw[] = {0x61,0xa8}; SoftwareWire::vocCrc_=SGP40::crc(raw,2);
  BoardIO board; board.setup(); hardware_api::Measurement value = {};
#if defined(TEST_CONFLICT)
  assert(busBegins==TEST_EXPECT_BUS_COUNT && board.voc(0,value)==hardware_api::NOT_IMPLEMENTED);
#elif defined(TEST_UNCONFIGURED)
  assert(busBegins==0 && gpioCalls==0);
  assert(board.voc(0,value)==hardware_api::NOT_IMPLEMENTED);
  assert(board.temperature(0,value)==hardware_api::NOT_IMPLEMENTED);
#else
  assert(busBegins==5 && gpioCalls==2);
  assert(board.voc(0,value)==hardware_api::SENSOR_UNAVAILABLE);
  assert(board.voc(2,value)==hardware_api::INVALID_INDEX);
  for(timeNow=0;timeNow<49000;++timeNow) {
    unsigned before=operations; board.poll(timeNow);
    assert(operations-before<=1); // общий budget: один датчик / loop
  }
  assert(board.voc(0,value)==hardware_api::OK && value.value>=1 && value.value<=500 && value.flags==3);
  assert(board.voc(0,value)==hardware_api::OK && value.flags==1);
  assert(board.voc(1,value)==hardware_api::OK && value.flags==3);
  bool closed=false;
  assert(board.door(0,closed)==hardware_api::OK && closed);
  assert(board.door(1,closed)==hardware_api::OK && !closed);
  assert(board.door(2,closed)==hardware_api::INVALID_INDEX);
#endif
  puts("sensor board: pin safety, SGP40 pair, freshness, round-robin budget passed");
}
