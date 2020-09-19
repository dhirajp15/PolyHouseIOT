/*
 * @file - lora_node.cpp
 * @brief - Node Modes of operations Implementation source file.
   * LoRa Node Functions
   * Initialization Mode
   * In Initialization Mode
   * 1. The Node scans for Gateway Id & config
   * 2. Once gateway id anc config is received , it will go to broadcasting mode
   * Broadcasting mode 
   * In Broadcasting mode , Node advertises its identity and type for every 10 secs 
   * The broadcasting mode completes once Initialization Successful message is received from Gateway 
   * Next , The Node will start performing the task based on role  
   * Sensor Mode 
   * Sensor Mode , the node keep sending periodic data to gateway in every T seconds 
   * Actuator Mode 
   * Actuator Mode, the node continuously llisten to Gateway commands and take necessary actions 
   * 
   * 
   * created 13 August 2020
   * by Dhiraj Patil
*/

#include <string.h>
#include <Arduino.h>
#include "lora_node.h"


//Configurations
NODE_ROLE role_of_node = SENSOR_NODE;
MODE mode_of_operation = INITIALIZE_MODE;
SENSOR_TYPE type_of_node = TEMP_HUMIDITY;

#define number_of_sensors 1
#define number_of_actuators 0

#define GATEWAY_ID "2000"

bool is_gateway_id_received = false;

extern void LoRa_sendMessage(String message); 

void initialization_begin(){
  //wait for Gateway Id
  Serial.println("Waiting for Gateway Id..");
  if(is_gateway_id_received == true){
    mode_of_operation = BROADCAST_MODE;
  }
}

void broadcast_begin(){
  Serial.println("Starting BroadCast");
  String broadcast_str = generate_message(NODE_ID,NODE_INIT,NO_HEADER,"TempSensor Node");
  Serial.print("BroadCast Message: ");
  Serial.println(broadcast_str);
  LoRa_sendMessage(broadcast_str); 
}

void sensor_mode_begin(){
  
}

void actuator_mode_begin(){
  
}

void update_node_state(lora_message msg){
  if(mode_of_operation == INITIALIZE_MODE){
    //validate gateway id
    if(msg.id == GATEWAY_ID){
      if(msg.op_code == GATEWAY_INIT){
        Serial.println("Initializing...");
        Serial.println("Init Message:");
        Serial.println(msg.mdata);
        is_gateway_id_received = true;
      }
      else{
         Serial.println("Opcode Mismatch");
      }
    }
    else{
      Serial.println("Gateway Id Mismatch");
    }
  }
}
