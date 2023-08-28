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
#define INIT_SUCCESS_MSG "SUCCESS"
#define SENSOR_REQ_MSG "SENSOR_COUNT: 2,SENSOR TYPE: TEMP_HUMMIDITY"
#define ACK_MSG   "GATEWAY_RECEIVED_ACK"
#define COM_PCK   "1"



const long frequency = 433E6;  // LoRa Frequency

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 14;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

bool is_node_id_received = false;
bool is_sensor_data_received = false;
bool is_command_acknowledged = false;
void print_hex(char* data, int len);

char rx_buf[256]; 
String msg_str = "";

void setup() {
  Serial.begin(115200);                   // initialize serial
  Serial.println("Begin done.");
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
  if (Serial.available()) {      // If anything comes in Serial (USB),
    char op = Serial.read();   // read it and send it out Serial1 (pins 0 & 1)
    switch (op){
      //Initialize 
      case '1':
          msg_str = generate_message(GATEWAY_ID,GATEWAY_INIT,NO_HEADER,INIT_DATA);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
          Serial.println("Sending Id");
          //Wait for getting Node Packet
          while(is_node_id_received == false){
            Serial.println("Waiting for Node Id...");
            delay(2000);
          }
          is_node_id_received = false;
          //Send Init successfuul packet
          msg_str = generate_message(GATEWAY_ID,INIT_SUCCESS,NO_HEADER,INIT_SUCCESS_MSG);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
       break;
       //Query Sensor Value
       case '2':
          msg_str = generate_message(GATEWAY_ID,SENSOR_DATA_REQ,NO_HEADER,SENSOR_REQ_MSG);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
          Serial.println("Sending Sensor Request");
          //Wait for getting Sensor Data
          while(is_sensor_data_received == false){
            Serial.println("Waiting for Sensor Data...");
            delay(2000);
          }
          is_sensor_data_received = false;
          //Send ack
          msg_str = generate_message(GATEWAY_ID,ACK,NO_HEADER,ACK_MSG);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
       break;
       //Send command packet to actuator node
       case '3':
          msg_str = generate_message(GATEWAY_ID,COMMAND,NO_HEADER,COM_PCK);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
          Serial.println("Sending Command");
          //Wait for Acknowledgement
          while(is_command_acknowledged == false){
            Serial.println("Waiting for Command Acknowledgement...");
            delay(2000);
          }
          is_command_acknowledged = false;
          //Send ack
          msg_str = generate_message(GATEWAY_ID,ACK,NO_HEADER,ACK_MSG);
          Serial.println(msg_str);
          LoRa_sendMessage(msg_str); // send a message
       break;
     default:
     break;
    }
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
  memset(rx_buf,0,256);
  int i = 0;
  while (LoRa.available()) {
    //Check if start byte is present
    if( (i == 2) && (!is_start_byte_present(rx_buf)) ) {
      Serial.print("Invalid Start Byte");
      break;
      }
    rx_buf[i] = (char)LoRa.read();
    i++;
  }
  Serial.print("Gateway Receive: ");
  Serial.print(" with RSSI ");
  Serial.println(LoRa.packetRssi());
  Serial.println(rx_buf);
  lora_message msg = parse_received_message(rx_buf,i);
  if(msg.op_code == NODE_INIT){
    Serial.print("Node ID Receive: ");
    Serial.println(msg.id);
    Serial.print("Node Message: ");
    Serial.println(msg.mdata);
    is_node_id_received = true;
  }
  else if(msg.op_code == SENSOR_DATA){
    Serial.print("Node ID Receive: ");
    Serial.println(msg.id);
    Serial.print("Sensor Data Header: ");
    Serial.println(msg.data_header);
    Serial.print("Sensor Data: ");
    Serial.println(msg.mdata);
    is_sensor_data_received = true;
  }
  else if(msg.op_code == ACK){
    Serial.print("Node ID Receive: ");
    Serial.println(msg.id);
    Serial.print("Command Acknowledged: ");
    Serial.println(msg.mdata);
    is_command_acknowledged = true;
  }
  else{
    Serial.print("Unknown Op code: ");
    Serial.println(msg.op_code);
  }
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
