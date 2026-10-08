#pragma once
#include <stdint.h>
// Один GP2Y1010AU0F: собственный кэш и период 1 с, без знания GPIO/протокола.
// ADC и напряжение — НЕ концентрация пыли и не доказательство подключения.
class GP2Y1010AU0F {
 public:
  class Reader {
   public:
    virtual ~Reader() {}
    virtual void begin() = 0;
    virtual bool sample(uint16_t& adc) = 0;
    virtual uint32_t completedAt(uint32_t now) const { return now; }
  };
  struct Reading { uint16_t adc, millivolts; bool valid, fresh; uint16_t ageMs; };
  explicit GP2Y1010AU0F(Reader* reader = nullptr, uint16_t referenceMv = 5000)
      : reader_(reader), referenceMv_(referenceMv) { setup(); }
  void setup(uint32_t now = 0) {
    adc_ = 0; valid_ = fresh_ = false; measuredAt_ = now; lastAttempt_ = now - 1000UL;
  }
  void attach(Reader& reader, uint16_t referenceMv, uint32_t now) {
    reader_ = &reader; referenceMv_ = referenceMv; setup(now); reader_->begin();
  }
  bool poll(uint32_t now) {
    if (!reader_ || !referenceMv_ || static_cast<uint32_t>(now-lastAttempt_) < 1000UL) return false;
    uint16_t adc = 0;
    const bool success = reader_->sample(adc);
    const uint32_t completed = reader_->completedAt(now);
    lastAttempt_ = completed;
    if (!success || adc > 1023) { fresh_ = false; return false; }
    adc_ = adc; measuredAt_ = completed; valid_ = fresh_ = true; return true;
  }
  Reading read(uint32_t now) {
    const uint32_t age = static_cast<uint32_t>(now-measuredAt_);
    Reading value = {adc_, static_cast<uint16_t>((static_cast<uint32_t>(adc_)*referenceMv_+511)/1023),
                     valid_, valid_ && fresh_, static_cast<uint16_t>(age>65534 ? 65534 : age)};
    fresh_ = false; return value;
  }
 private:
  Reader* reader_;
  uint16_t referenceMv_, adc_;
  uint32_t lastAttempt_, measuredAt_;
  bool valid_, fresh_;
};
