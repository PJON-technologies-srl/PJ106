
/* PJ106 LED module configuration console Giovanni Blu Mitolo 2020
 This is a basic example to show how PJ106 can be used practically.
 The Serial monitor is used to configure and print data from PJ106 */

#define PJON_PACKET_MAX_LENGTH 20
#define PJON_MAX_PACKETS 2
#include <PJONSoftwareBitBang.h>

PJONSoftwareBitBang bus(PJON_MASTER_ID);

uint8_t packet[20];
String string_number;
int req_index = 0;
uint8_t recipient_id = PJON_NOT_ASSIGNED;

void setup() {
  bus.strategy.set_pin(12);
  bus.begin();
  bus.set_receiver(receiver_function);
  bus.set_error(error_handler);
  Serial.begin(115200);
  Serial.println("P102 Temperature and humidity sensor configuration console - Giovanni Blu Mitolo 2020");
  print_help();
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
  Serial.print(" PJ106 module info: ");
  for(uint8_t i = 0; i < 5; i++)
    Serial.print((char)payload[i]);
  Serial.print(" - Software version: ");
  Serial.println(payload[5], DEC);
};

void error_handler(uint8_t code, uint8_t data) {
  if(code == PJON_CONNECTION_LOST) {
    Serial.print("Connection lost - PJ106 ID ");
    Serial.println((uint8_t)bus.packets[data].content[0]);
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    Serial.print("Packet buffer is full, has now a length of ");
    Serial.println(data, DEC);
    Serial.println("Possible wrong bus configuration!");
    Serial.println("higher PJON_MAX_PACKETS if necessary.");
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    Serial.print("Content is too long, length: ");
    Serial.println(data);
  }
};

void print_help() {
  Serial.println();
  Serial.println("Input commands guide: ");
  Serial.println("?!        -> Help and device info");
  Serial.println("H!        -> LED on");
  Serial.println("L!        -> LED off");
  Serial.println("I1-254!   -> Configure module id");
  Serial.println("R0-255!   -> Configure recipient id");
  Serial.println("T0-65535! -> Configure blink interval");
  Serial.println("Q!        -> Block incoming configuration (caution)");
  Serial.println("X!        -> Configuration reset");
  Serial.println();
  Serial.println("Use the Serial console input field to configure the PJ106 module.");
  Serial.println("All commands must end with !");
  Serial.println();
}

void loop() {
  bus.receive(1000);
  bus.update();

  if(Serial.available() > 0) {
    char req_byte = Serial.read();
    packet[req_index] = (uint8_t)req_byte;
    if(req_index && req_byte != '!')
      string_number += req_byte;

    if(req_byte == '!') {
      uint16_t result = 0;
      uint8_t packet_length = 0;
      // LED on
      if((char)packet[0] == 'H') {
        bus.send(recipient_id, "H",1);
        Serial.println("PJ106 LED on");
        Serial.flush();
      }
      // LED off
      if((char)packet[0] == 'L') {
        bus.send(recipient_id, "L",1);
        Serial.println("PJ106 LED off");
        Serial.flush();
      }
      // Block further incoming configuration
      if((char)packet[0] == 'Q') {
        packet_length = 1;
        bus.send(recipient_id, packet, packet_length);
        Serial.println("Blocking further incoming configuration!");
        Serial.println("Flash the PJ106 module with PJ106.ino to unblock!");
        Serial.print(packet[1]);
        Serial.flush();
      }
      // Recipient id update
      if((char)packet[0] == 'R') {
        packet[1] = (uint8_t)string_number.toInt();
        packet_length = 2;
        bus.send(recipient_id, packet, packet_length);
        Serial.println("Recipient id update: ");
        Serial.print(packet[1]);
        bus.set_id(packet[1]);
        Serial.flush();
      }
      // Interval configuration update
      if((char)packet[0] == 'T')  {
        packet[1] = string_number.toInt() >> 8;
        packet[2] = string_number.toInt() & 0xFF;
        packet_length = 3;
        bus.send(recipient_id, packet, packet_length);
        Serial.print("Setting transmission interval: ");
        Serial.println(string_number.toInt());
        Serial.flush();
      }
      // Configuration reset to default
      if((char)packet[0] == 'X') {
        packet_length = 1;
        bus.send(recipient_id, packet, packet_length);
        Serial.print("Executing configuration reset!");
        Serial.flush();
      }
      // Print Help
      if((char)packet[0] == '?') {
        print_help();
        bus.send(recipient_id, "?", 1);
      }

      req_index = 0;
      string_number = "";
      return;
    }
    req_index++;
  }
};
