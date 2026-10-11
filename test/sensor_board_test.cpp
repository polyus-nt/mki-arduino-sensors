#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
#define NUM_DIGITAL_PINS 20
#define NUM_ANALOG_INPUTS 6
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
static uint32_t timeNow=0;
unsigned long millis() {return timeNow;}
static unsigned gpioCalls=0;
void pinMode(int, int) {++gpioCalls;}
int digitalRead(int pin) {return pin == 11 ? LOW : HIGH;}
// Подменяем только электрический транспорт: production BoardIO и драйверы настоящие.
#define SoftwareWire_h
static unsigned busBegins=0, operations=0;
void digitalWrite(int, int) {}
void delayMicroseconds(unsigned int) {}
int analogRead(int pin) { ++operations; return pin == A0 ? 512 : 1023; }
// Подменяем DHT только на электрической границе. Кэш/период DHT11 настоящие.
#define DHT_H
static const uint8_t DHT_TYPE_11 = 11;
static unsigned dhtBegins=0, dhtReads=0;
class DHT {
 public:
  DHT(uint8_t pin, uint8_t type) : pin_(pin) { assert(type==11); }
  void begin() {++dhtBegins;}
  float readHumidity(bool force) {
    assert(force); ++dhtReads; ++operations;
    return pin_==13 ? 55.5f : pin_==18 ? 80.0f : 100.0f;
  }
 private:
  uint8_t pin_;
};
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
#if defined(TEST_DHT_VOC_CONFLICT)
  assert(busBegins==3 && gpioCalls==2 && dhtBegins==0);
  assert(board.voc(0,value)==hardware_api::NOT_IMPLEMENTED);
  assert(board.humidity(0,value)==hardware_api::NOT_IMPLEMENTED);
#elif defined(TEST_CONFLICT)
  assert(busBegins==TEST_EXPECT_BUS_COUNT && board.voc(0,value)==hardware_api::NOT_IMPLEMENTED);
#elif defined(TEST_UNCONFIGURED)
  assert(busBegins==0 && gpioCalls==0);
  assert(board.voc(0,value)==hardware_api::NOT_IMPLEMENTED);
  assert(board.temperature(0,value)==hardware_api::NOT_IMPLEMENTED);
  assert(board.humidity(0,value)==hardware_api::NOT_IMPLEMENTED);
#elif defined(TEST_UNO_MAP)
  assert(API_LINK_MISO == 9 && API_LINK_MOSI == 10);
  assert(API_LINK_CLK == 11 && API_LINK_CS == 12 && API_LINK_IRQ == 13);
  assert(API_DHT0_PIN == A3 && API_DHT1_PIN == A4 && API_DHT2_PIN == A5);
  assert(API_VOC0_SDA_PIN == 7 && API_VOC1_SDA_PIN == 8);
  assert(API_VOC0_SCL_PIN == 6 && API_VOC1_SCL_PIN == 6);
  assert(API_DUST0_LED_PIN == A2);
  assert(API_DUST0_ANALOG_PIN == A1);
  assert(busBegins == 5 && dhtBegins == 3 && gpioCalls == 2);
  assert(board.temperature(0,value) == hardware_api::SENSOR_UNAVAILABLE);
  for(timeNow=0;timeNow<49000;++timeNow) {
    unsigned before=operations; board.poll(timeNow);
    assert(operations-before<=1);
  }
  assert(board.voc(0,value)==hardware_api::OK);
  assert(board.voc(1,value)==hardware_api::OK);
  assert(board.humidity(0,value)==hardware_api::OK);
  assert(board.dust(value)==hardware_api::OK && value.value==((1023UL<<16)|5000));
  bool closed=false;
  assert(board.door(0,closed)==hardware_api::NOT_IMPLEMENTED);
  assert(board.door(1,closed)==hardware_api::NOT_IMPLEMENTED);
#else
  assert(busBegins==5 && gpioCalls==2);
#if defined(API_DHT_CONFIGURED) && !defined(TEST_DHT_CONFLICT)
  assert(dhtBegins==3 && board.humidity(0,value)==hardware_api::SENSOR_UNAVAILABLE);
#else
  assert(dhtBegins==0 && board.humidity(0,value)==hardware_api::NOT_IMPLEMENTED);
#endif
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
#if defined(API_DHT_CONFIGURED) && !defined(TEST_DHT_CONFLICT)
  const uint32_t humidity[] = {5550,8000,10000};
  for(uint8_t i=0;i<3;++i) {
    assert(board.humidity(i,value)==hardware_api::OK && value.value==humidity[i] && value.flags==3);
    assert(board.humidity(i,value)==hardware_api::OK && value.flags==1);
  }
  assert(dhtReads>3 && board.humidity(3,value)==hardware_api::INVALID_INDEX);
#endif
#endif
  puts("sensor board: pin safety, SGP40/DHT11 caches, freshness, round-robin budget passed");
}
