// датчик Холла

// #define KY_024_DO_PIN 13
// #define AO_PIN A0
static uint8_t ky24_do_pin = 13;

void setup_hall(int digital_pin) {
  ky24_do_pin = digital_pin;
  pinMode(ky24_do_pin, INPUT);
}

int read_hall() {
  return digitalRead(ky24_do_pin);
}
