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

void setup() {
  Serial.begin(9600);
  if (!validLinkPins()) { Serial.println(F("invalid Austere pin map")); return; }
  board.setup();
  linkReady = link.setup();
}

void loop() {
  if (!linkReady) return;
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
  // Сначала транспорт, затем максимум одна I²C-операция BMP280/SGP40 в idle-итерации.
  if (!receivedByte && !replies.pending()) board.poll(millis());
}
