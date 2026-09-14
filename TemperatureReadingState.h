#pragma once

// Hardware-independent validity state.  A failed conversion (including a
// scratchpad CRC error) never manufactures a first reading and never makes a
// retained reading fresh.
class TemperatureReadingState {
 public:
  struct Value {
    float value;
    bool valid;
    bool fresh;
  };

  TemperatureReadingState() : value_(0.0f), valid_(false) {}

  Value retained() const { return {value_, valid_, false}; }

  Value complete(bool crcValid, float value) {
    if (!crcValid) return retained();
    value_ = value;
    valid_ = true;
    return {value_, true, true};
  }

 private:
  float value_;
  bool valid_;
};
