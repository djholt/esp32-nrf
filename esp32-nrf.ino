#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include "RF24.h"

#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#define PAYLOAD_SIZE 32

bool transmitter = true;
uint8_t address[][8] = { "Node001", "Node002" };
int counter = 1;

RF24 radio(0, 22); // CE, CSN

void printToScreen(String s) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.println(s);
  display.display();
}

void setup() {
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  printToScreen("Hello!");
  delay(500);

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, CS
  if (!radio.begin()) {
    printToScreen("Radio not responding!");
    while (true);
  }

  radio.setPALevel(RF24_PA_HIGH);
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

    delay(50);

    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);
    radio.startListening();

    delay(1000); // wait 1 second for the reply to be received

    if (radio.available()) {
      char payload[PAYLOAD_SIZE];
      radio.read(&payload, radio.getPayloadSize());
      printToScreen("Received reply:\n" + String(payload));
    } else {
      printToScreen("No reply!");
    }

    delay(250);

    radio.openWritingPipe(address[0]);
    radio.openReadingPipe(1, address[1]);
    radio.stopListening();

    delay(2000);

  } else { // RX

    char payload[PAYLOAD_SIZE];

    if (radio.available()) {
      radio.read(&payload, radio.getPayloadSize());
      printToScreen("Received:\n" + String(payload));
    }

    delay(50);

    radio.openWritingPipe(address[0]);
    radio.openReadingPipe(1, address[1]);
    radio.stopListening();

    String message = "GOT " + String(payload);
    const char *message_cstr = message.c_str();

    if (radio.write(message_cstr, strlen(message_cstr))) {
      printToScreen("Sent: " + message);
    } else {
      printToScreen("Send failed: " + message);
    }

    delay(250);

    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);
    radio.startListening();
  }
}