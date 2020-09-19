/**
 ****************************************************************************************
 *
 * @file lora_node.h
 *
 * @brief Node Modes of operations Implementation header file.
 *
 *
 * created 13 August 2020
 * by Dhiraj Patil
 ****************************************************************************************
 */

 
#ifndef _LORA_NODE_
#define _LORA_NODE_

#include "lora_packet.h"

//defines

#define NODE_ID "1101" 
#define RX_BUF_LEN 256 

//LoRa
#define CS_PIN    10    //LoRa radio chip select
#define RESET_PIN 9     //LoRa radio reset
#define IRQ_PIN   2     //change for your board; must be a hardware interrupt pin

//Sensors & Actuators
#define DHTPIN 3
#define DHTTYPE DHT11

//enums

enum MODE{
  INITIALIZE_MODE,
  BROADCAST_MODE,
  ACTIVE_MODE,
  SENSOR_MODE,
  ACTUATOR_MODE
};

enum SENSOR_TYPE{
  TEMP_HUMIDITY,
  MOISTURE,
  PIR,
  LIGHT
};

void initialization_begin();
void broadcast_begin();
void sensor_mode_begin();
void actuator_mode_begin();
void active_mode_begin();
void update_node_state(lora_message msg);
String get_sensor_data(SENSOR_TYPE sensor);















#endif // _LORA_NODE_
