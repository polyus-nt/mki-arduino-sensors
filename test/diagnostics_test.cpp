#include "mock/Arduino.h"
#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include "../src/SensorDiagnostics.h"
struct Output {
  std::ostringstream text;
  template<class T> void print(T v) { text << v; }
  void print(uint8_t v) { text << static_cast<unsigned>(v); }
  void print(uint8_t v, int base) { print(static_cast<uint16_t>(v), base); }
  void print(uint16_t v, int base) { text << (base == HEX ? std::hex : std::dec) << v << std::dec; }
  void print(double v, int digits) { text << std::fixed << std::setprecision(digits) << v; }
  template<class T> void println(T v) { print(v); println(); }
  void println() { text << '\n'; }
};
int main() {
  using namespace hardware_api;
  Frame request = {}; request.version=kVersion; request.kind=kRequest;
  request.node=kSensorsNode; request.command=kGetDust; request.idx=1;
  request.tx_id=1; request.session_id=UINT64_C(0x123456789abcdef0);
  uint8_t wire[kFrameSize]; encode(request,wire);
  StreamParser parser; Frame received={};
  for(unsigned i=0;i<kFrameSize;++i) assert(parser.feed(wire[i],i,received)==(i==kFrameSize-1));
  Frame reply=response(received); writeU16(reply.data,512); writeU16(reply.data+2,2502);
  reply.data[4]=3; writeU16(reply.data+5,123);
  Output out; printSensorReply(out,reply);
  assert(out.text.str().find("Dust ADC: 512; voltage: 2.502 V")!=std::string::npos);
  assert(out.text.str().find("valid=1 fresh=1 age_ms=123")!=std::string::npos);
  encode(reply,wire); std::ostringstream expected;
  expected << "TX HEX: ";
  for(unsigned i=0;i<kFrameSize;++i) {
    if(i) expected << ' ';
    expected << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << unsigned(wire[i]);
  }
  // Arduino Print HEX использует верхний регистр; mock — нижний.
  std::string actual=out.text.str(), hex=expected.str();
  for(char& ch:actual) if(ch>='a' && ch<='f') ch-=32;
  assert(actual.find(hex)!=std::string::npos);
  reply.command=kGetTemp; writeU32(reply.data,static_cast<uint32_t>(-1234));
  Output temp; printSensorReply(temp,reply);
  assert(temp.text.str().find("Temperature: -12.34 C")!=std::string::npos);
  reply.status=SENSOR_UNAVAILABLE;
  Output error; printSensorReply(error,reply);
  assert(error.text.str().find("Temperature:")==std::string::npos);
  assert(error.text.str().find("status=5")!=std::string::npos);
}
