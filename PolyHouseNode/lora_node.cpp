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
MODE mode_of_operation = INITIALIZE_MODE;
SENSOR_TYPE type_of_node = TEMP_HUMIDITY;

#define number_of_sensors 1
#define number_of_actuators 0

#define GATEWAY_ID "2000"

extern void LoRa_sendMessage(String message); 

void initialization_begin(){
  //wait for Gateway Id
  Serial.println("Waiting for Gateway Id..");
}

void broadcast_begin(){
  Serial.println("Starting BroadCast");
  String broadcast_str = generate_message(NODE_ID,NODE_INIT,NO_HEADER,"TempSensor Node");
  Serial.print("BroadCast Message: ");
  Serial.println(broadcast_str);
  LoRa_sendMessage(broadcast_str); 
}

void active_mode_begin(){
  //wait for Gateway Instructions
  Serial.println("Listening to Gateway..");
}
  

void sensor_mode_begin(){
  //Sending Sensor Data Information
  Serial.println("Sending Sensor Data to Gateway");
  String dht_sensor_value_str =get_sensor_data(TEMP_HUMIDITY);
  Serial.println(dht_sensor_value_str);
  String sensor_data_str = generate_message(NODE_ID,SENSOR_DATA,TEMP_S,(char*)dht_sensor_value_str.c_str());
  Serial.println(sensor_data_str);
  LoRa_sendMessage(sensor_data_str); 
}

void actuator_mode_begin(){
  
}

void update_node_state(lora_message msg){
      //validate gateway id
      if(msg.id == GATEWAY_ID){
        switch (msg.op_code){
          case GATEWAY_INIT:
            Serial.println("Initializing...");
            Serial.println("Init Message:");
            Serial.println(msg.mdata);
            mode_of_operation = BROADCAST_MODE;
          break;
          case INIT_SUCCESS:
            Serial.println("Initialization Successful");
            Serial.println(msg.mdata);
            mode_of_operation = ACTIVE_MODE;
          break;
          case SENSOR_DATA_REQ:
            Serial.println("Sensor Data Request Received");
            Serial.println(msg.mdata);
             mode_of_operation = SENSOR_MODE;
           break;
           case ACK:
            Serial.println("Received Acknowledgement");
            Serial.println(msg.mdata);
             mode_of_operation = ACTIVE_MODE;
          default:
           Serial.println("Opcode Mismatch");
          break;
        }
      }
      else{
        Serial.println("Gateway Id Mismatch");
      }
}
