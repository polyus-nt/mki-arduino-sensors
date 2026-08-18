// Датчик-геркон K-021

void setup_gerkon_ky_021(int pin) {
  pinMode(pin, INPUT);     // Настраиваем пин датчика на вход
}

int read_gerkon_ky_021(int pin) {
    int sensorState = digitalRead(pin); // Читаем состояние датчика
    if (sensorState == LOW) {               // Если магнит рядом (контакты замкнуты)
      Serial.println("Магнит обнаружен!");
      return 1;
    } else {
      Serial.println("Магнита нет");
      return 0;                                // Если магнита нет
    }
    // delay(100);                             // Небольшая задержка для стабильности
}
