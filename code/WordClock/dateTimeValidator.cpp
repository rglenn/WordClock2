#include "dateTimeValidator.h"
#include <Arduino.h>

static const uint8_t daysInMonth[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint8_t validateDate(uint16_t yearNum, uint8_t monthNum, uint8_t dayNum)
{
  uint8_t dimOffset = 0;
  
  if((yearNum < 2000) || (yearNum > 2100)) {
    return 2;
  }

  if((monthNum < 1) || (monthNum > 12)) {
    return 3;
  }

  // Leap year handling
  if(monthNum == 2) {
    if(yearNum % 4 == 0) {
      if(yearNum % 100 == 0) {
        if(yearNum % 400 == 0) {
          dimOffset = 1; //is a leap year
        } 
        else {
          dimOffset = 0; //not a leap year
        } 
      } 
      else {
        dimOffset = 1; //is a leap year
      }
    } 
    else {
      dimOffset = 0;// not a leap year
    } 
  }

  if((dayNum < 1) || (dayNum > (daysInMonth[monthNum - 1] + dimOffset))) {
    return 4;
  }
  return 0;
}

uint8_t validateTime(uint8_t hourNum, uint8_t minNum, uint8_t secNum)
{
  if((hourNum < 0) || (hourNum > 23)) {
    return 2;
  }

  if((minNum < 0) || (minNum > 59)) {
    return 3;
  }

  if((secNum < 0) || (secNum > 59)) {
    return 4;
  }
}
