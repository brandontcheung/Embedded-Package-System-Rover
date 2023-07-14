#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_
#include <stdio.h>
#include <stdint.h>
//#include "jsmn.h"
#include <string.h>
// JAKE PAN OCT.23


int jsonParser_MQTT(const char *JSON_STRING, char *bufferRecipient, char *bufferSender, char *bufferMsgID, char *bufferBody ,char *bufferElse);

//int jsonParser_MQTT(char *JSON_STRING, char bufferMsgID, char bufferBoardID, char bufferMessage);


#endif //JSON_parser_H
