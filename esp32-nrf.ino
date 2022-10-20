#include "heltec.h"
#include "RF24.h"
#include <SPI.h>

#define PAYLOAD_SIZE 32

bool transmitter = true;
uint8_t address[][8] = { "Node001", "Node002" };
int counter = 1;

RF24 radio(0, 22); // CE, CSN

void printToScreen(String s) {
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, s);
  Heltec.display->display();
}

void setup() {
  Heltec.begin(true /*display*/, false /*LoRa*/, true /*Serial*/);
  printToScreen("READY");

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, CS
  if (!radio.begin()) {
    printToScreen("Radio not responding!");
    while (true);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(PAYLOAD_SIZE);
  if (transmitter) {
    radio.openWritingPipe(address[0]);
    radio.openReadingPipe(1, address[1]);
    radio.stopListening();
  } else {
    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);
    radio.startListening();
  }
}

void loop() {
  if (transmitter) { // TX

    String message = "test" + String(counter++, DEC);
    const char *message_cstr = message.c_str();

    if (radio.write(message_cstr, strlen(message_cstr))) {
      printToScreen("Sent: " + message);
    } else {
      printToScreen("Send failed: " + message);
    }

    delay(250);

  } else { // RX

    if (radio.available()) {
      char payload[PAYLOAD_SIZE];
      radio.read(&payload, radio.getPayloadSize());
      printToScreen("Received: " + String(payload));
    }

  }
}
