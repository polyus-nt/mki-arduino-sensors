void setup() {
  pinMode(A0, INPUT_PULLUP);     // Настраиваем пин датчика на вход
  // pinMode(ledPin, OUTPUT);     // Настраиваем пин светодиода на выход
  Serial.begin(9600);          // Запускаем монитор порта для отладки
}

void loop() {
  int sensorState = analogRead(A0); // Читаем состояние датчика
  Serial.println(sensorState);
  // if (sensorState == LOW) {               // Если магнит рядом (контакты замкнуты)
    // digitalWrite(ledPin, HIGH);          // Включаем светодиод
    // Serial.println("Магнит обнаружен!");
  // } else {                                // Если магнита нет
    // digitalWrite(ledPin, LOW);           // Выключаем светодиод
    // Serial.println("Магнита нет");
  // }
  delay(100);                             // Небольшая задержка для стабильности
}