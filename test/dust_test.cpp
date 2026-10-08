#include <assert.h>
#include <stdio.h>
#include <vector>
#define A0 14
#define A1 15
#define NUM_DIGITAL_PINS 20
#define NUM_ANALOG_INPUTS 6
#include "mock/Arduino.h"
static uint32_t now = 0;
static int raw = 512;
struct Event { int kind, value; };
static std::vector<Event> events;
unsigned long millis() { return now; }
void pinMode(int, int) {}
void digitalWrite(int, int v) { events.push_back({0,v}); }
int digitalRead(int) { return HIGH; }
int analogRead(int pin) { events.push_back({2,pin}); return raw; }
void delayMicroseconds(unsigned int us) { events.push_back({1,static_cast<int>(us)}); }
#include "../src/BoardIO.h"
int main() {
  GP2Y1010Hardware gpio(7,A0); gpio.begin(); events.clear();
  uint16_t adc = 0; assert(gpio.sample(adc) && adc == 512);
  assert(events.size()==5);
  assert(events[0].kind==0 && events[0].value==LOW);
  assert(events[1].kind==1 && events[1].value==280);
  assert(events[2].kind==2 && events[2].value==A0);
  assert(events[3].kind==1 && events[3].value==40);
  assert(events[4].kind==0 && events[4].value==HIGH);
  raw=1024; assert(!gpio.sample(adc)); assert(events.back().value==HIGH);
  raw=512; BoardIO board; board.setup();
  hardware_api::Measurement value={};
#ifdef TEST_DUST_CONFLICT
  assert(events.size()==10); // лишь прямой тест gpio; BoardIO не трогает конфликтующие пины
  assert(board.dust(0,value)==hardware_api::NOT_IMPLEMENTED);
#else
  assert(board.dust(0,value)==hardware_api::SENSOR_UNAVAILABLE);
  for(unsigned i=0;i<10;++i) board.poll(now);
  assert(board.dust(0,value)==hardware_api::OK);
  assert(value.value==((512UL<<16)|2502) && value.flags==3);
  assert(board.dust(0,value)==hardware_api::OK && value.flags==1);
  assert(board.dust(1,value)==hardware_api::OK && value.flags==3);
  assert(board.dust(2,value)==hardware_api::INVALID_INDEX);
  now=1000;raw=1024;for(unsigned i=0;i<10;++i)board.poll(now);
  assert(board.dust(0,value)==hardware_api::OK && value.flags==1 && value.ageMs==1000);
  now=70000;assert(board.dust(0,value)==hardware_api::OK && value.ageMs==65534);
  raw=1023;for(unsigned i=0;i<10;++i)board.poll(now);
  hardware_api::Frame request={};request.version=hardware_api::kVersion;
  request.kind=hardware_api::kRequest;request.node=13;request.command=hardware_api::kGetDust;
  request.session_id=UINT64_C(0x123456789ABCDEF0);request.tx_id=65535;
  hardware_api::SensorApi api(board);const hardware_api::Frame reply=api.handle(request);
  assert(reply.status==hardware_api::OK && hardware_api::sameRequest(request,reply));
  assert(hardware_api::readU16(reply.data)==1023 && hardware_api::readU16(reply.data+2)==5000);
  assert(reply.data[4]==3 && hardware_api::readU16(reply.data+5)==0 && reply.data[7]==0);
  GP2Y1010AU0F sensor(&gpio,3300);now=0xfffffff0UL;sensor.setup(now);
  assert(sensor.poll(now));now=10;assert(!sensor.poll(now));
  now=1000;assert(sensor.poll(now));assert(sensor.read(now).millivolts==3300);
#endif
  puts("GP2Y1010: LED timing, two caches, ADC/mV, errors, age, wrap, pins, byte response passed");
}
