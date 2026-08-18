// Плата ардуино, которая получает данные с 
// Датчик температуры KY-001
// Датчик температуры Dallas DS18B20
// Датчик Холла KY-024
// Датчик Геркона KY-025

#include "AustereSerialMaster.h"
#include "./Led.h"
#include "./DS18B20.h"
#include "./KY-024.h"
#include "./KY-025.h"
#include "./KY-021.h"

#define KY024_DO_PIN 12 
#define KY025_DO_PIN 13
#define KY025_AO_PIN A1
#define KY021_D0_PIN 10


DS18B20 insideTemperatureSensor(8);
DS18B20 outsideTemperatureSensor(9);

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
int gerkon_values[3] = { 0, 0, 0 };

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
  setup_gerkon(KY025_DO_PIN);
  setup_gerkon(KY025_AO_PIN);
  setup_gerkon_ky_021(KY021_D0_PIN);
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
      temperature = insideTemperatureSensor.readTemperature();
      temperature2 = outsideTemperatureSensor.readTemperature();
      hall_value = read_hall();
      gerkon_values[0] = read_gerkon(A0);
      gerkon_values[1] = read_gerkon(A1);
      gerkon_values[2] = read_gerkon_ky_021(KY021_D0_PIN);
      // после опроса отправляем ok
      link.sendByte(OK_CODE);
    }
    if (b == SEND_CODE) {
      // отправляем результаты счетчиков
      link.sendByte(hall_value);
      link.sendByte(gerkon_values[0]);
      link.sendByte(gerkon_values[1]);
      link.sendByte(gerkon_values[2]);
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
