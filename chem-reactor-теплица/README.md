# Arduino датчиков теплицы-химреактора

Отдельная прошивка в `arduino-sensor`, фаблаб-скетч не меняется.
BMP280×2 (температура/давление), DHT11×2 (влажность), MQ-2×2 (ADC/напряжение).
Концевики находятся на МС-ТЮК.

Откройте [arduino-sensors.ino](arduino-sensors/arduino-sensors.ino) в Arduino IDE,
выберите **Arduino Uno** и нужный USB-порт. SoftwareWire, DHT и Austere
включены в src, соседние репозитории и дополнительные библиотеки не нужны.
Вся карта: [BoardConfig.h](arduino-sensors/BoardConfig.h).

| Назначение | Пины UNO |
| --- | --- |
| BMP280 общий SCL | D4 |
| BMP280 SDA, idx0/1 | D5 / D6 |
| DHT11, idx0/1 | D2 / D3 |
| MQ-2 ADC, idx0/1 | A1 / A0 |
| Austere MISO / MOSI / CLK / CS / IRQ | D9 / D10 / D11 / D12 / D13 |

MQ-2 выдаёт ADC и напряжение с принятой опорой 5000 мВ, не ppm.
Прогрев/калибровка не реализованы; корректный ADC не доказывает готовность датчика.

Для TEST раскомментируйте #define TEST в BoardConfig и загрузите скетч.
Serial Monitor: 9600, Newline. Команды:

```text
temp 0
pressure 1
humidity 0
gas 1
help
```

Печатаются status, значение, fresh/age и байты ответа. Austere продолжает работать.
Нативные проверки из этой папки: `py -3 scripts/test_firmware.py --cxx g++`.
Они не требуют других репозиториев.
Полная карта/протокол: `tjc-sketch/chem-reactor-теплица/PINOUT.md` и PROTOCOL.md.
Сервис/HTTP-консоль: `tjc-commutator/chem-reactor-теплица/`.
