/*
 * @file - lora_node.cpp
 * @brief - LORA Communication Packet Implementation source file.
   * Packet Format
   * Start - Start of Packet
   * ID - Gateway/Sensor Identification Number
   * Op Code - Indicate whether it is initialization, ,command , data or acknowledgement packet 
   * Data Header - Information on type of data present
   * Data Length - Length of data
   * Data - Data/Command Value 
   * CRC - Cyclic Redundancy Check
   * END - End of Packet
   * 
   * created 13 August 2020
   * by Dhiraj Patil
*/
#include <string.h>
#include <stdlib.h>
#include "lora_packet.h"


String generate_message(const char* id, OP_CODE op_code, DATA_HEADER data_header, char* data_cstr){
  String msg = "";
  uint8_t start_byte = START_BYTE;
  uint8_t end_byte = END_BYTE;
  msg += convert_hex_to_string((char*)&start_byte,1);
  msg += id;
  msg += convert_hex_to_string((char*)&op_code,1);
  msg += convert_hex_to_string((char*)&data_header,1);
  uint16_t data_length = strlen(data_cstr);
  msg += convert_hex_to_string((char*)&data_length,sizeof(uint16_t));
  msg += data_cstr;
  msg += convert_hex_to_string((char*)&end_byte,1);
  return(msg);
}

lora_message parse_received_message(String received_message_str, int msg_len){
  //todo: validate crc
  lora_message received_msg;
  received_msg.start_byte = strtol(received_message_str.substring(0, 2).c_str(), NULL, 16);
  Serial.print("start_byte:");
  Serial.println(received_msg.start_byte,HEX);
  received_msg.id = received_message_str.substring(2, 6);
  Serial.print("GateWay Id:");
  Serial.println(received_msg.id);
  received_msg.op_code = (OP_CODE)atoi(received_message_str.substring(6, 8).c_str());
  Serial.print("OP Code:");
  Serial.println(received_msg.op_code);
  received_msg.data_header = (DATA_HEADER)atoi(received_message_str.substring(8, 10).c_str());
  Serial.print("Data Header:");
  Serial.println(received_msg.data_header);
  int length_L = strtol(received_message_str.substring(10,12).c_str(), NULL, 16);
  int length_H = strtol(received_message_str.substring(12,14).c_str(), NULL, 16);
  received_msg.data_length = length_H*100 + length_L;
  Serial.print("Data Length:");
  Serial.println(received_msg.data_length);
  received_msg.mdata = received_message_str.substring(14,14+received_msg.data_length);
  Serial.print("Data:");
  Serial.println(received_msg.mdata);
  received_msg.end_byte = strtol(received_message_str.substring(14+received_msg.data_length,received_msg.data_length+16).c_str(), NULL, 16);
  Serial.print("End Byte:");
  Serial.println(received_msg.end_byte,HEX);
  return(received_msg);
}

String convert_hex_to_string(char* hex_cstr ,int len){
  String hex_str = "";
  char temp[3];
  for(int i=0; i<len ; i++)
  {
    memset(temp,0,3);
    sprintf(temp,"%.2X",hex_cstr[i]);
    hex_str+= temp[0];
    hex_str+= temp[1];
  }
  return(hex_str);
}

bool is_start_byte_present(char* rx_buf){
  uint8_t start_byte = strtol(rx_buf, NULL, 16);
  Serial.print("start_byte: ");
  Serial.println(start_byte,HEX);
  if(start_byte != START_BYTE){
    return false;
  }
  return true;
}
