// Плата ардуино, которая получает данные с 
// Датчик температуры KY-001
// Датчик температуры Dallas DS18B20
// Датчик Холла KY-024
// Датчик Геркона KY-025

#include "AustereSerialMaster.h"
#include "./Led.h"
#include "./DS18B20.h"
#include "./KY-024.h"
#include "./Koltsevik.h"

#define KY024_DO_PIN 12
#define DEBUG false
static const uint8_t LIMIT_SWITCH_0_PIN = A0;
static const uint8_t LIMIT_SWITCH_1_PIN = A1;
static const uint8_t LIMIT_SWITCH_2_PIN = A2;


DS18B20 insideTemperatureSensor(A3);
DS18B20 outsideTemperatureSensor(A4);

// Pick any digital pins per spec.
static const uint8_t PIN_MISO = 2;
static const uint8_t PIN_MOSI = 3;
static const uint8_t PIN_CLK  = 4;
static const uint8_t PIN_CS   = 5;
static const uint8_t PIN_IRQ  = 6;

AustereSerialMaster link(PIN_MISO, PIN_MOSI, PIN_CLK, PIN_CS, PIN_IRQ);

// --- НАСТРОЙКА ПИНОВ ---
// const int RELAY_PIN = 8;     // Пин управления реле (включает/выключает +12В)
// const int FAN_PWM_PIN = {9 };   // Синий провод кулера (управление скоростью)
// const int TACH_PIN = { 2 };      // Желтый/Зеленый провод кулера (тахометр) - ТОЛЬКО D2!

static const uint8_t POLLING_CODE = 1;
static const uint8_t ACTUATOR_CODE = 2;
static const uint8_t OK_CODE = 3;
static const uint8_t SEND_CODE = 4;


float temperature = 0;
float temperature2 = 0;
int hall_value = 0;
int limit_switch_values[3] = { 0, 0, 0 };

Koltsevik limitSwitch0(LIMIT_SWITCH_0_PIN);
Koltsevik limitSwitch1(LIMIT_SWITCH_1_PIN);
Koltsevik limitSwitch2(LIMIT_SWITCH_2_PIN);

void printLinkByteForTest(uint8_t byteValue);

// USB Serial diagnostic. Call it from loop() while testing the wiring.
// It does not send anything through link and does not change its protocol.
void testSensorsAndLink() {
  uint8_t byteValue;
  while (link.recvByte(&byteValue)) {
    printLinkByteForTest(byteValue);
  }

  static uint32_t lastTestMs = 0;
  const uint32_t now = millis();
  if (now - lastTestMs < 2000UL) {
    return;
  }
  lastTestMs = now;

  const bool insideConnected = insideTemperatureSensor.isConnected();
  const bool outsideConnected = outsideTemperatureSensor.isConnected();
  const float insideTemperature = insideTemperatureSensor.readTemperature();
  const float outsideTemperature = outsideTemperatureSensor.readTemperature();

  Serial.println(F("--- Sensor test ---"));
  Serial.print(F("DS18B20 inside: "));
  Serial.print(insideConnected ? F("OK, ") : F("NOT FOUND, "));
  Serial.println(insideTemperature, 2);
  Serial.print(F("DS18B20 outside: "));
  Serial.print(outsideConnected ? F("OK, ") : F("NOT FOUND, "));
  Serial.println(outsideTemperature, 2);
  Serial.print(F("Hall (D12): "));
  Serial.println(read_hall());
  Serial.print(F("Limit switches raw/state A0, A1, A2: "));
  Serial.print(limitSwitch0.rawValue());
  Serial.print('/');
  Serial.print(limitSwitch0.read());
  Serial.print(F(", "));
  Serial.print(limitSwitch1.rawValue());
  Serial.print('/');
  Serial.print(limitSwitch1.read());
  Serial.print(F(", "));
  Serial.print(limitSwitch2.rawValue());
  Serial.print('/');
  Serial.println(limitSwitch2.read());
}

void printLinkByteForTest(uint8_t byteValue) {
  Serial.print(F("Link RX: 0x"));
  if (byteValue < 0x10) {
    Serial.print('0');
  }
  Serial.println(byteValue, HEX);
}

// Format: [int8_t whole part][uint8_t fraction normalized to 0..255].
// The fraction represents hundredths of a degree (0..99).
void sendTemperature(float value) {
  value = constrain(value, -128.0f, 127.99f);

  // floorf preserves the sign for values between -1.00 and 0.00.
  int8_t wholePart = (int8_t)floorf(value);
  uint8_t hundredths = (uint8_t)lroundf((value - wholePart) * 100.0f);

  // For example, 23.999 becomes 24.00 after rounding to hundredths.
  if (hundredths == 100) {
    wholePart++;
    hundredths = 0;
  }

  const uint8_t normalizedFraction =
      (uint8_t)lroundf((float)hundredths * 255.0f / 99.0f);

  link.sendByte((uint8_t)wholePart);
  link.sendByte(normalizedFraction);
}

void setup() {
  Serial.begin(9600);
  insideTemperatureSensor.setup();
  outsideTemperatureSensor.setup();
  link.setup();
  Serial.println("AustereSerialMaster ready");
  limitSwitch0.begin();
  limitSwitch1.begin();
  limitSwitch2.begin();
  setup_hall(KY024_DO_PIN);

  Serial.println("Датчики ready");
}

void do_numbers() {
  // // Example TX once per second.
  // static uint32_t lastTxMs = 0;
  // static uint8_t counter = 0;
  // const uint32_t now = millis();
  // if (now - lastTxMs >= 1000) {
  //   lastTxMs = now;
  //   link.sendByte(counter++);
  // }

  // Drain received bytes.
  uint8_t b;
  while (link.recvByte(&b)) {
    // printLinkByteForTest(b);
    if (b == POLLING_CODE) {
      // опрос всех датчиков
      temperature = insideTemperatureSensor.readTemperature();
      temperature2 = outsideTemperatureSensor.readTemperature();
      hall_value = read_hall();
      limit_switch_values[0] = limitSwitch1.read();
      limit_switch_values[1] = limitSwitch2.read();
      limit_switch_values[2] = limitSwitch0.read();
      // после опроса отправляем ok
      link.sendByte(OK_CODE);
    }
    if (b == SEND_CODE) {
      // отправляем результаты счетчиков
      link.sendByte(hall_value);
      link.sendByte(limit_switch_values[0]);
      link.sendByte(limit_switch_values[1]);
      link.sendByte(limit_switch_values[2]);
      sendTemperature(temperature);
      sendTemperature(temperature2);
      link.sendByte(OK_CODE);
    }
    if (b == ACTUATOR_CODE) {
      // зажечь светодиод, уйдет на плату-актуатор
      // led.toggle();
      link.sendByte(OK_CODE);
    }
  }
}

// 0d 46 46 fe 0a 0d

void loop() {
  if (DEBUG) {
    testSensorsAndLink();
  } else {
    do_numbers();
  }
}
