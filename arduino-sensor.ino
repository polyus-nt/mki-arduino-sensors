#include <Arduino.h>
#include "BoardConfig.h"
#include "src/austere/AustereSerialMaster.h"
#include "src/BoardIO.h"
#include "src/LinkPins.h"

BoardIO board;
hardware_api::SensorApi api(board);
AustereSerialMaster link(API_LINK_MISO, API_LINK_MOSI, API_LINK_CLK, API_LINK_CS, API_LINK_IRQ);
hardware_api::StreamParser parser;
hardware_api::ReplyBuffer replies;
uint16_t lastOverruns = 0;
bool linkReady = false;
#ifdef TEST
#include "src/SensorDiagnostics.h"
#include "src/SensorTestInput.h"
SensorTestInput serialInput;
#endif

void setup() {
  Serial.begin(9600);
  if (!validLinkPins()) { Serial.println(F("invalid Austere pin map")); return; }
  board.setup();
  linkReady = link.setup();
#ifdef TEST
  printSensorHelp(Serial);
#endif
}

void loop() {
  if (!linkReady) return;
#ifdef TEST
  // Отдельный парсер: фрагменты Serial и Austere никогда не смешиваются.
  // Один байт за итерацию, чтобы не вытеснять аппаратный транспорт/опрос.
  serialInput.tick(millis());
  if (Serial.available() > 0) {
    hardware_api::Frame serialRequest;
    const SensorTestInput::Event event=serialInput.feed(static_cast<uint8_t>(Serial.read()), millis(), serialRequest);
    if (event==SensorTestInput::REQUEST)
      printSensorReply(Serial, api.handle(serialRequest));
    else if (event==SensorTestInput::HELP) printSensorHelp(Serial);
    else if (event==SensorTestInput::ERROR)
      Serial.println(F("Invalid command/frame. Type help; check index and CRC for binary input."));
  }
#endif
  parser.tick(millis());
  if (link.overrunCount() != lastOverruns) {
    lastOverruns = link.overrunCount(); parser.reset(); link.clearRx();
    Serial.println(F("Austere RX overflow: partial request discarded"));
  }
  bool receivedByte = false;
  if (!replies.pending()) {
    uint8_t byte;
    hardware_api::Frame request;
    if (link.recvByte(&byte)) {
      receivedByte = true;
      if (parser.feed(byte, millis(), request) && request.kind == hardware_api::kRequest)
        replies.enqueue(api.handle(request));
    }
  }
  if (replies.pending() && link.sendByte(replies.nextByte())) replies.accepted();
  // Сначала транспорт, затем один BMP280/SGP40/DHT11/GP2Y в idle-итерации.
  if (!receivedByte && !replies.pending()) board.poll(millis());
}
