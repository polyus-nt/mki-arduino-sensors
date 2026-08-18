// Геркон

// #define DO_PIN 13
#define AO_PIN A0

static uint8_t ky25_do_pin = 13; 

void setup_gerkon(int pin) {
  ky25_do_pin = pin;
  pinMode(ky25_do_pin, INPUT);
}

int read_gerkon(int pin) {
  int analogVal = analogRead(pin);

  // Serial.print("KY-025 Digital: ");
  // Serial.println(digitalVal);
  Serial.print("Gerkon Analog: ");
  Serial.println(analogVal);
  if (analogVal > 10) return 0;
  else return 1;
  // return digitalVal;
}