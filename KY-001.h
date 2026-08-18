// Датчик температуры
#include "OneWire.h"

float read_temperature(int not_found_delay, OneWire ds) {
    byte i;
    byte data[12];
    byte addr[8];
    float celsius;
    // поиск датчика
    if ( !ds.search(addr)) {
        ds.reset_search();
        // Serial.println("NOT FOUND!");
        // delay(not_found_delay);
        return;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // измерение температуры
    delay(100);
    ds.reset();
    ds.select(addr); 
    ds.write(0xBE); // начало чтения измеренной температуры
    //показания температуры из внутренней памяти датчика
    for ( i = 0; i < 9; i++) {
        data[i] = ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    // датчик может быть настроен на разную точность, выясняем её 
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) {
      raw = raw & ~7; // точность 9-разрядов, 93,75 мс
    }
    else if (cfg == 0x20) {
      raw = raw & ~3; // точность 10-разрядов, 187,5 мс
    }  
    else if (cfg == 0x40) {
      raw = raw & ~1; // точность 11-разрядов, 375 мс
    }
    // преобразование показаний в градусы Цельсия 
    celsius = (float)raw / 16.0;
    
    return celsius;
}
