#pragma once
#include <Arduino.h>
// Раскомментировать для диагностики через Serial (9600 бод).
// #define TEST
#ifdef HARDWARE_API_BOARD_HEADER
#include HARDWARE_API_BOARD_HEADER
#else
// Согласованная карта Arduino Uno. A2–A5 — цифровые GPIO датчиков.
#define API_LINK_MISO 9
#define API_LINK_MOSI 10
#define API_LINK_CLK 11
#define API_LINK_CS 12
#define API_LINK_IRQ 13
#define API_BMP_SCL_PIN 2
#define API_BMP_SDA0_PIN 3
#define API_BMP_SDA1_PIN 4
#define API_BMP_SDA2_PIN 5
#define API_VOC0_SCL_PIN 6
#define API_VOC1_SCL_PIN 6
#define API_VOC0_SDA_PIN 7
#define API_VOC1_SDA_PIN 8
#define API_DHT0_PIN A2
#define API_DHT1_PIN A3
#define API_DHT2_PIN A4
#define API_DUST0_LED_PIN A5
#define API_DUST1_LED_PIN A5
#define API_DUST0_ANALOG_PIN A0
#define API_DUST1_ANALOG_PIN A1
// Концевики перенесены на кнопочницу МС-ТЮК; D0/D1 остаются для Serial.
#endif
// Единственное место настройки распиновки, видимое вкладкой Arduino IDE.
// Внешняя карта выше используется для compile-only проверок; пины остаются
// в config-файлах, а не передаются числовыми флагами компилятора.
// Транспорт переносится только полной группой из пяти пинов.
#if defined(API_LINK_MISO) || defined(API_LINK_MOSI) || defined(API_LINK_CLK) || defined(API_LINK_CS) || defined(API_LINK_IRQ)
#if !defined(API_LINK_MISO) || !defined(API_LINK_MOSI) || !defined(API_LINK_CLK) || !defined(API_LINK_CS) || !defined(API_LINK_IRQ)
#error "define all five API_LINK_* pins together"
#endif
#else
#define API_LINK_MISO 2
#define API_LINK_MOSI 3
#define API_LINK_CLK 4
#define API_LINK_CS 5
#define API_LINK_IRQ 6
#endif
// BMP280 x3: общий SCL, три отдельные SDA, адрес каждого 0x76.
// SoftwareWire с исправленным тайм-аутом находится в src/softwarewire.
// #define API_BMP_SCL_PIN 2
// #define API_BMP_SDA0_PIN 3
// #define API_BMP_SDA1_PIN 4
// #define API_BMP_SDA2_PIN 5
#if defined(API_BMP_SCL_PIN) || defined(API_BMP_SDA0_PIN) || defined(API_BMP_SDA1_PIN) || defined(API_BMP_SDA2_PIN)
#if !defined(API_BMP_SCL_PIN) || !defined(API_BMP_SDA0_PIN) || !defined(API_BMP_SDA1_PIN) || !defined(API_BMP_SDA2_PIN)
#error "define all four BMP280 bus pins"
#endif
#define API_BMP_CONFIGURED
#endif
// SGP40 x2: отдельные SDA; разрешён общий SCL. Обмен последовательный.
// #define API_VOC0_SDA_PIN ...
// #define API_VOC0_SCL_PIN ...
// #define API_VOC1_SDA_PIN ...
// #define API_VOC1_SCL_PIN ...
#if defined(API_VOC0_SDA_PIN) || defined(API_VOC0_SCL_PIN) || defined(API_VOC1_SDA_PIN) || defined(API_VOC1_SCL_PIN)
#if !defined(API_VOC0_SDA_PIN) || !defined(API_VOC0_SCL_PIN) || !defined(API_VOC1_SDA_PIN) || !defined(API_VOC1_SCL_PIN)
#error "define all four SGP40 bus pins"
#endif
#define API_VOC_CONFIGURED
#endif
// Концевики: два отдельных входа, INPUT_PULLUP. -1 означает отключённый вход;
// уровень CLOSED_LEVEL задаётся по физическому монтажу (LOW либо HIGH).
// Входы концевиков не могут пересекаться с датчиками и Austere.
// #define API_DOOR0_PIN ...
// #define API_DOOR1_PIN ...
// #define API_DOOR0_CLOSED_LEVEL LOW
// #define API_DOOR1_CLOSED_LEVEL LOW
#ifndef API_DOOR0_PIN
#define API_DOOR0_PIN -1
#endif
#ifndef API_DOOR1_PIN
#define API_DOOR1_PIN -1
#endif
#ifndef API_DOOR0_CLOSED_LEVEL
#define API_DOOR0_CLOSED_LEVEL LOW
#endif
#ifndef API_DOOR1_CLOSED_LEVEL
#define API_DOOR1_CLOSED_LEVEL LOW
#endif
// DHT11 x3: каждый вход уникален во всей карте.
// #define API_DHT0_PIN 8
// #define API_DHT1_PIN 9
// #define API_DHT2_PIN 10
#if defined(API_DHT0_PIN) || defined(API_DHT1_PIN) || defined(API_DHT2_PIN)
#if !defined(API_DHT0_PIN) || !defined(API_DHT1_PIN) || !defined(API_DHT2_PIN)
#error "define all three DHT11 pins"
#endif
#define API_DHT_CONFIGURED
#endif
// GP2Y1010AU0F x2: для каждого LED active LOW и аналоговый вход.
// Разрешён общий LED; аналоговые входы остаются отдельными.
// #define API_DUST0_LED_PIN ...
// #define API_DUST0_ANALOG_PIN A0
// #define API_DUST1_LED_PIN ...
// #define API_DUST1_ANALOG_PIN A1
#ifndef API_DUST_REFERENCE_MV
#define API_DUST_REFERENCE_MV 5000
#endif
#if API_DUST_REFERENCE_MV < 1 || API_DUST_REFERENCE_MV > 65535
#error "ADC reference must be 1..65535 millivolts"
#endif
#if defined(API_DUST0_LED_PIN) || defined(API_DUST0_ANALOG_PIN) || defined(API_DUST1_LED_PIN) || defined(API_DUST1_ANALOG_PIN)
#if !defined(API_DUST0_LED_PIN) || !defined(API_DUST0_ANALOG_PIN) || !defined(API_DUST1_LED_PIN) || !defined(API_DUST1_ANALOG_PIN)
#error "define all four GP2Y1010AU0F pins"
#endif
#define API_DUST_CONFIGURED
#endif
