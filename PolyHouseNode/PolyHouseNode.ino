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
  
  Modified 13 August 2020
  by Dhiraj Patil
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "lora_node.h"
#include "lora_packet.h"

extern MODE mode_of_operation;

const long frequency = 433E6;  // LoRa Frequency

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

char rx_buf[RX_BUF_LEN]; 

void print_hex(char* data, int len);
void LoRa_sendMessage(String message);

DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);

  LoRa.setPins(CS_PIN, RESET_PIN, IRQ_PIN);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  //Initialize Sensor Devices.
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
}

void loop() {
  if (runEvery(5000)) {
     switch (mode_of_operation){
      case INITIALIZE_MODE:
        //check if gateway id and config received 
        initialization_begin();
      break;
      case BROADCAST_MODE:
        broadcast_begin();
      break;
      case ACTIVE_MODE:
        active_mode_begin();
      break;
      case SENSOR_MODE:
        sensor_mode_begin();
      break;
      case ACTUATOR_MODE:
        actuator_mode_begin();
      break;
      default:
      break;
     }
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  memset(rx_buf,0,RX_BUF_LEN);
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
    Serial.print("Node Receive: ");
    Serial.println(rx_buf);
    lora_message msg = parse_received_message(rx_buf,i);
    Serial.println(msg.mdata);
    update_node_state(msg);
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

String get_sensor_data(SENSOR_TYPE sensor){
  String sensor_data = "";
  switch(sensor){
      case TEMP_HUMIDITY:
         // Get temperature event and send its value to LoRa Gateway
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature)) {
          Serial.println(F("Error reading temperature!"));
          sensor_data = "Error reading temperature";
          break;
        }
        else {
        Serial.print(F("Temperature: "));
        Serial.print(event.temperature);
        Serial.println(F("°C"));
        }
        sensor_data += String(event.temperature)+" deg C ,";
        // Get humidity event and print its value.
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity)) {
          Serial.println(F("Error reading humidity!"));
          sensor_data = "Error reading humidity";
          break;
         }
        else {
          Serial.print(F("Humidity: "));
          Serial.print(event.relative_humidity);
          Serial.println(F("%"));
        }
        sensor_data += String(event.relative_humidity)+" %";
      break;
  }
  return(sensor_data);
}

void switch_on_led(String mesg){
  const int ledPin =  LED_BUILTIN;// the number of the LED pin
  int ledState = LOW;             // ledState used to set the LED
  pinMode(ledPin, OUTPUT);
  if(mesg == '1'){
    ledState= HIGH;
  }
}
