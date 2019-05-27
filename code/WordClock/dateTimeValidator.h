#ifndef SURREALITYLABS_DATETIME_VALIDATOR
#define SURREALITYLABS_DATETIME_VALIDATOR

#include <Arduino.h>

uint8_t validateDate(uint16_t yearNum, uint8_t monthNum, uint8_t dayNum);
uint8_t validateTime(uint8_t hourNum, uint8_t minNum, uint8_t secNum);

#endif
