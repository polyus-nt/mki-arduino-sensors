#pragma once
// ТОЛЬКО проверка кода BMP280 x3, SGP40 x2 и двух концевиков на Uno.
// Карта не пересекается с транспортом, но НЕ согласована для стенда!
// Никогда не используйте её как предположение о реальном подключении.
#define API_BMP_SCL_PIN 14
#define API_BMP_SDA0_PIN 15
#define API_BMP_SDA1_PIN 16
#define API_BMP_SDA2_PIN 17
#define API_VOC0_SDA_PIN 7
#define API_VOC0_SCL_PIN 8
#define API_VOC1_SDA_PIN 9
#define API_VOC1_SCL_PIN 10
#define API_DOOR0_PIN 11
#define API_DOOR1_PIN 12
