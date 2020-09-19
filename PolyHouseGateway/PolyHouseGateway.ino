/*
  LoRa Simple Gateway/Node Exemple

  This code uses InvertIQ function to create a simple Gateway/Node logic.

  Gateway - Sends messages with enableInvertIQ()
          - Receives messages with disableInvertIQ()

  Node    - Sends messages with disableInvertIQ()
          - Receives messages with enableInvertIQ()

  With this arrangement a Gateway never receive messages from another Gateway
  and a Node never receive message from another Node.
  Only Gateway to Node and vice versa.

  This code receives messages and sends a message every second.

  InvertIQ function basically invert the LoRa I and Q signals.

  See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
  for more on InvertIQ register 0x33.

  created 05 August 2018
  by Luiz H. Cassettari
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include "lora_packet.h"

#define GATEWAY_ID "2000"
#define INIT_DATA "Interval,30min,RTCtime,1600005124"


const long frequency = 433E6;  // LoRa Frequency

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 14;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

bool is_messages_received = false;
void print_hex(char* data, int len);

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  if (runEvery(10000)) { // repeat every 5000 millis
    //if(is_messages_received == true){
    String msg_str = generate_message(GATEWAY_ID,GATEWAY_INIT,NO_HEADER,INIT_DATA);
    Serial.println(msg_str);
    LoRa_sendMessage(msg_str); // send a message
    Serial.println("Sending Id");
    //}
  }
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  is_messages_received = true;
  Serial.print("Gateway Receive: ");
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
  Serial.println(message);
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


void print_hex(char* data, int len){
  Serial.println("Value in hex:");
  for(int i=0; i<len ; i++){
  Serial.print(data[i], HEX);
  Serial.print(" ");
  if((i+1)%8 == 0)
    Serial.println("");
  }
}
