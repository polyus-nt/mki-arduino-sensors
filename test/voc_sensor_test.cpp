#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../SGP40.h"
#include "../src/SensorApi.h"
struct Bus : SGP40::Bus {
  uint16_t raw = 25000;
  bool sendOk = true, readOk = true, badCrc = false;
  unsigned sends = 0, reads = 0;
  uint32_t duration = 0;
  uint32_t completedAt(uint32_t now) const override { return now + duration; }
  bool send(const uint8_t* data, uint8_t count) override {
    const uint8_t expected[] = {0x26,0x0f,0x80,0x00,0xa2,0x66,0x66,0x93};
    assert(count == 8 && memcmp(data, expected, 8) == 0); ++sends; return sendOk;
  }
  bool receive(uint8_t* data, uint8_t count) override {
    assert(count == 3); ++reads;
    data[0] = raw >> 8; data[1] = raw; data[2] = SGP40::crc(data, 2) ^ badCrc;
    return readOk;
  }
};
static void sample(SGP40& sensor, uint32_t now) {
  sensor.poll(now); sensor.poll(now + 34); sensor.poll(now + 35);
}
struct Hardware : hardware_api::SensorHardware {
  uint32_t index=100;
  hardware_api::Status temperature(uint8_t, hardware_api::Measurement&) override {return hardware_api::NOT_IMPLEMENTED;}
  hardware_api::Status pressure(uint8_t, hardware_api::Measurement&) override {return hardware_api::NOT_IMPLEMENTED;}
  hardware_api::Status door(uint8_t, bool&) override {return hardware_api::NOT_IMPLEMENTED;}
  hardware_api::Status unsupported(uint8_t, uint8_t) override {return hardware_api::NOT_IMPLEMENTED;}
  hardware_api::Status voc(uint8_t idx, hardware_api::Measurement& value) override {
    value = {index + idx, 3, 35}; return hardware_api::OK;
  }
};
int main() {
  const uint8_t humidity[] = {0x80,0x00}, temperature[] = {0x66,0x66};
  assert(SGP40::crc(humidity,2) == 0xa2 && SGP40::crc(temperature,2) == 0x93);
  Bus a,b; b.raw = 30000; SGP40 first(&a), second(&b);
  for (uint32_t i=0; i<46; ++i) {
    sample(first,i*1000); sample(second,i*1000);
    assert(!first.read(i*1000+35).valid && !second.read(i*1000+35).valid);
  }
  sample(first,46000); sample(second,46000);
  auto result = first.read(46035);
  assert(result.valid && result.fresh && result.value >= 1 && result.value <= 500 && result.ageMs == 0);
  assert(!first.read(46035).fresh);
  assert(second.read(46035).valid);
  first.poll(46999); assert(a.sends == 47);
  a.badCrc = true; sample(first,47000);
  result = first.read(47035); assert(result.valid && !result.fresh && result.ageMs == 1000 && !first.isConnected());
  sample(second,47000); assert(second.read(47035).fresh); // сосед не сброшен
  a.badCrc=false; sample(first,48000); assert(!first.read(48035).fresh);
  assert(first.read(200000).ageMs == 65534);
  Bus failure; failure.sendOk=false; SGP40 failed(&failure);
  failed.poll(0); failed.poll(35); assert(failure.reads == 0 && !failed.read(35).valid);
  failure.sendOk=true; failure.readOk=false; sample(failed,1000); assert(!failed.read(1035).valid);
  failure.readOk=true; failure.raw=0; sample(failed,2000); assert(!failed.isConnected());
  failure.raw=65000; sample(failed,3000); assert(!failed.isConnected());
  failure.raw=65535; sample(failed,4000); assert(!failed.isConnected());
  Bus wrap; SGP40 wrapped(&wrap); wrapped.setup(0xfffffff0UL);
  wrapped.poll(0xfffffff0UL); wrapped.poll(18); assert(wrap.reads==0);
  wrapped.poll(19); assert(wrap.reads==1);
  wrapped.poll(984); assert(wrap.sends==2);
  Bus missed; SGP40 late(&missed); late.poll(0); late.poll(1000);
  assert(missed.reads==0 && !late.read(1000).valid);
  late.poll(3000); late.poll(3035); assert(missed.sends==2 && missed.reads==1);
  Bus stretched; stretched.duration=20; SGP40 timed(&stretched);
  timed.poll(0); timed.poll(35); assert(stretched.reads==0);
  timed.poll(54); assert(stretched.reads==0);
  timed.poll(55); assert(stretched.reads==1); // 35 мс от конца send
  Bus jitter; SGP40 stable(&jitter);
  for(uint32_t i=0;i<48;++i) sample(stable,i*1005);
  assert(stable.read(47*1005+35).valid); // обычный джиттер не вызывает reset
  const auto previous=stable.read(47*1005+35);
  sample(stable,49*1005); // пропуск >=2000мс: сохранить cache, заново blackout
  const auto stale=stable.read(49*1005+35);
  assert(stale.valid && !stale.fresh && stale.value==previous.value && stale.ageMs==2010);
  jitter.raw=65000; sample(stable,50*1005);
  assert(!stable.read(50*1005+35).fresh);
  Hardware hardware; hardware_api::SensorApi api(hardware);
  hardware_api::Frame request = {};
  request.version=1; request.kind=1; request.node=13; request.command=hardware_api::kGetVoc;
  for(uint8_t idx=0;idx<2;++idx) {
    request.idx=idx; auto reply=api.handle(request);
    assert(reply.status==hardware_api::OK && hardware_api::readU32(reply.data)==100U+idx);
    assert(reply.data[4]==3 && hardware_api::readU16(reply.data+5)==35 && reply.data[7]==0);
  }
  request.idx=2; assert(api.handle(request).status==hardware_api::INVALID_INDEX);
  request.idx=0;
  hardware.index=0; assert(api.handle(request).status==hardware_api::INTERNAL_ERROR);
  hardware.index=501; assert(api.handle(request).status==hardware_api::INTERNAL_ERROR);
  hardware.index=500; assert(api.handle(request).status==hardware_api::OK);
  printf("SGP40: CRC, independent warmup, cache, cadence, errors, wrap, VOC wire passed; Params=%zu bytes\n",sizeof(GasIndexAlgorithmParams));
}
