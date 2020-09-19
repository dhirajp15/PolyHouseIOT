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

lora_message parse_received_message(char* received_message, int msg_len){
  //todo: validate crc
  
  lora_message received_msg;
  
  memcpy(&received_msg,received_message,msg_len);
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
