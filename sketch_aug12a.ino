// Плата ардуино, которая получает данные с 
// Датчик температуры KY-001
// Датчик температуры и влажности DHT11
// Датчик Холла KY-024
// Датчик Геркона KY-025

#include "AustereSerialMaster.h"
#include "./Led.h"
#include "./KY-001.h"
#include "./DH-11.h"
#include "./KY-024.h"
#include "./KY-025.h"

#define KY024_DO_PIN 12 
#define KY025_DO_PIN 13


#include <OneWire.h>

OneWire ds(8);
DH11 dht11(11);

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


static const LED led = LED(8);
float temperature = 0;
float temperature2 = 0;
int hall_value = 0;
int gerkon_value = 0;

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
  dht11.setup();
  link.setup();
  Serial.println("AustereSerialMaster ready");
  setup_gerkon(KY025_DO_PIN);
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
    if (b == POLLING_CODE) {
      // опрос всех датчиков
      temperature = read_temperature(25, ds);
      temperature2 = dht11.readTemperature();
      hall_value = read_hall();
      gerkon_value = read_gerkon(A0);
      // после опроса отправляем ok
      link.sendByte(OK_CODE);
    }
    if (b == SEND_CODE) {
      // отправляем результаты счетчиков
      link.sendByte(hall_value);
      link.sendByte(gerkon_value);
      sendTemperature(temperature);
      sendTemperature(temperature2);
      link.sendByte(OK_CODE);
    }
    if (b == ACTUATOR_CODE) {
      // зажечь светодиод, уйдет на плату-актуатор
      led.toggle();
      link.sendByte(OK_CODE);
    }
  }
}

// 0d 46 46 fe 0a 0d

void loop() {
  do_numbers();
}
