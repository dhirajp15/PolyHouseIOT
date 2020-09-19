#ifndef _LORA_PACKET_
#define _LORA_PACKET_

#include <stdint.h>
#include <Arduino.h>

#define START_BYTE 0x10
#define END_BYTE 0x16

#define COMMAND_DATA_LENGTH 2
#define ID_LENGTH 4

typedef enum OP_CODE{
  GATEWAY_INIT,
  NODE_INIT,
  COMMAND,
  SENSOR_DATA,
  ACK
}OP_CODE;

typedef enum DATA_HEADER{
  TEMP_S, //Temperature sensor
  LIGHT_S, //LIGHT Sensor
  MOISTURE_S, //Moisture Sensor
  PIR_S, //PIR Sensor
  AC1, //Actuator 1
  AC2, //Actuator 2
  NO_HEADER //Not Applicable
}DATA_HEADER;

typedef struct lora_message_header{
  uint8_t start_byte;
  uint8_t id[ID_LENGTH+1];
  OP_CODE op_code;
  DATA_HEADER data_header;
  uint16_t data_length;
}lora_message_header;

#define MESSAGE_HEADER_LEN sizeof(lora_message_header)

typedef struct lora_message{
  lora_message_header header;
  char* mdata;
  uint8_t end_byte;
}lora_message;

String generate_message(const char* id, OP_CODE op_code, DATA_HEADER data_header, char* data);
lora_message parse_received_message(char* received_message, int msg_len);
String convert_hex_to_string(char* hex_cstr ,int len);

#endif  //_LORA_PACKET_
