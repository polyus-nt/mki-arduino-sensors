// KY-021 reed switch.

void setup_gerkon_ky_021(int pin) {
  pinMode(pin, INPUT);
}

int read_gerkon_ky_021(int pin) {
  return digitalRead(pin) == LOW ? 1 : 0;
}
