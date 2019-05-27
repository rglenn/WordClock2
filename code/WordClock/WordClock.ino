#include "CommandShell.h"
#include "dateTimeValidator.h"
CommandShell CommandLine;

#include <RTClock.h>
#include <TimeLib.h>
#include <WS2812B.h>

#include <Timezone.h>   // https://github.com/JChristensen/Timezone

#include <MicroNMEA.h>
char gpsBuffer[85];
MicroNMEA nmea(gpsBuffer, sizeof(gpsBuffer));
#define GPSSerial Serial2
#define GPS_INTERVAL (60 * 1000ul)
uint32_t lastGPS;

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240};    // Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};     // Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

RTClock rtclock (RTCSEL_LSE);
WS2812B strip = WS2812B(128);

int setDateFunc(char * args[], char num_args);
int setTimeFunc(char * args[], char num_args);
int printTimeFunc(char * args[], char num_args);
int gpsInfoFunc(char * args[], char num_args);
int stripTestFunc(char *args[], char num_args);

/* UART command set array, customized commands may add here */
commandshell_cmd_struct_t uart_cmd_set[] =
{
  {
    "setDate", "\tsetDate [day] [month] [year]", setDateFunc      }
  ,
  {
    "setTime", "\tsetTime [hours] [minutes] [seconds]", setTimeFunc      }
  ,
  {
    "printTime", "\tprintTime", printTimeFunc      }
  ,
  {
    "gpsInfo", "\tgpsInfo", gpsInfoFunc      }
  ,
  {
    "stripTest", "\tstripTest", stripTestFunc
  },
  {
    0,0,0      }
};

int setDateFunc(char * args[], char num_args) {
  if(3 != num_args) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  uint8_t dayNum = atoi(args[0]);
  uint8_t monthNum = atoi(args[1]);
  uint16_t yearNum = atoi(args[2]);

  uint8_t tmp = validateDate(yearNum, monthNum, dayNum);

  if(tmp == 2) {
    Serial.println(F("Invalid year!"));
    return 2;
  } else if(tmp == 3) {
    Serial.println(F("Invalid month!"));
    return 3;
  } else if(tmp == 4) {
    Serial.println(F("Invalid day!"));
    return 4;
  }
  
  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  newTime.Year = CalendarYrToTm(yearNum);
  newTime.Month = monthNum;
  newTime.Day = dayNum;
  
  tmp_t = makeTime(newTime);
  tmp_t = myTZ.toUTC(tmp_t);
  rtclock.setTime(tmp_t);
  setTime(tmp_t);

  Serial.print(F("Setting date to "));
  Serial.print(dayNum);
  Serial.print('/');
  Serial.print(monthNum);
  Serial.print('/');
  Serial.println(yearNum);
  return 0;  
}

int setTimeFunc(char * args[], char num_args) {
  if(3 != num_args) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  int hourNum = atoi(args[0]);
  int minNum = atoi(args[1]);
  int secNum = atoi(args[2]);

  uint8_t tmp = validateTime(hourNum, minNum, secNum);

  if(tmp == 2) {
    Serial.println(F("Invalid hours!"));
    return 2;
  } else if(tmp == 3) {
    Serial.println(F("Invalid minutes!"));
    return 3;
  } else if(tmp == 4) {
    Serial.println(F("Invalid seconds!"));
    return 4;
  }

  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  newTime.Hour = hourNum;
  newTime.Minute = minNum;
  newTime.Second = secNum;
  tmp_t = makeTime(newTime);
  tmp_t = myTZ.toUTC(tmp_t);
  rtclock.setTime(tmp_t);
  setTime(tmp_t);

  Serial.print(F("Setting time to "));
  Serial.print(hourNum);
  Serial.print(F(":"));
  Serial.print(minNum);
  Serial.print(F(":"));
  Serial.println(secNum);
  return 0;  
}

int printTimeFunc(char * args[], char num_args) {
  Serial.print(F("The current time is:"));

  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  Serial.print(tmYearToCalendar(newTime.Year), DEC);
  Serial.print('/');
  Serial.print(newTime.Month, DEC);
  Serial.print('/');
  Serial.print(newTime.Day, DEC);
  Serial.print(' ');
  Serial.print(newTime.Hour, DEC);
  Serial.print(':');
  Serial.print(newTime.Minute, DEC);
  Serial.print(':');
  Serial.print(newTime.Second, DEC);
  Serial.println();
  return 0;
}

int gpsInfoFunc(char *args[], char num_args) {
  if (nmea.isValid()) {
    Serial.println("GPS has fix");
  } else {
    Serial.println("GPS does not have fix");
  }
  Serial.print("GPS Date/time is: ");
  Serial.print(nmea.getYear());
  Serial.print('-');
  Serial.print(int(nmea.getMonth()));
  Serial.print('-');
  Serial.print(int(nmea.getDay()));
  Serial.print('T');
  Serial.print(int(nmea.getHour()));
  Serial.print(':');
  Serial.print(int(nmea.getMinute()));
  Serial.print(':');
  Serial.println(int(nmea.getSecond()));
  Serial.print("GPS last synced at mills() value ");
  Serial.println(lastGPS);
  Serial.print("Current millis() value is ");
  Serial.println(millis());
  Serial.print("GPS sync interval is ");
  Serial.println(GPS_INTERVAL);
  return 0;
}

int stripTestFunc(char *args[], char num_args) {
  Serial.println("Testing red...");
  colorWipe(strip.Color(255, 0, 0), 20); // Red
  Serial.println("Testing green...");
  colorWipe(strip.Color(0, 255, 0), 20); // Red
  Serial.println("Testing blue...");
  colorWipe(strip.Color(0, 0, 255), 20); // Red
  Serial.println("Done");
  return 0;
}

void colorWipe(uint32_t c, uint8_t wait) 
{
  for(uint16_t i=0; i<strip.numPixels(); i++) 
  {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

time_t getHwTime(void) {
  return rtclock.getTime();
}

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Starting"));

  strip.begin();
  for(uint8_t i = 0; i < 128; i++)
    strip.setPixelColor(i, 0ul);
  strip.show();

  setTime(getHwTime());
  setSyncProvider(getHwTime);
  setSyncInterval(600);

  randomSeed(now());
  CommandLine.commandTable = uart_cmd_set;
  CommandLine.init(&Serial);

  GPSSerial.begin(9600);
}

void loop(void) {
  static uint32_t lastRun;
  tmElements_t nowTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, nowTime);
  static bool clockIsSet = false;

  CommandLine.runService();

  // Time rendering code
  if (millis() - lastRun > 500)
  {
    lastRun = millis();
    RenderTime(nowTime);
    strip.show();
  }

  while (GPSSerial.available()) {
    char c = GPSSerial.read();
    if (nmea.process(c)) {
      if (nmea.isValid()) {
        if ((millis() - lastGPS >  GPS_INTERVAL) || (!clockIsSet)) {
          nowTime.Year = CalendarYrToTm(nmea.getYear());
          nowTime.Month = nmea.getMonth();
          nowTime.Day = nmea.getDay();
          nowTime.Hour = nmea.getHour();
          nowTime.Minute = nmea.getMinute();
          nowTime.Second = nmea.getSecond();

          tmp_t = makeTime(nowTime);
          rtclock.setTime(tmp_t);
          setTime(tmp_t);
  
          lastGPS = millis();
          clockIsSet = true;
        }
      }
    }
  }
}

void RenderTime(tmElements_t now) {
  uint8_t i, thisHour;
  uint8_t amPm = 0;

  for(i=0; i<128; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }

  thisHour = now.Hour;

  // Set the minute LEDs
  i = now.Minute % 5;
  switch(i) {
  case 4:
    #ifdef DEBUG
    Serial.print("4 ");
    #endif
    strip.setPixelColor(127, strip.Color(0, 128, 128));
  case 3:
    #ifdef DEBUG
    Serial.print("3 ");
    #endif
    strip.setPixelColor(71, strip.Color(0, 128, 128));
  case 2:
    #ifdef DEBUG
    Serial.print("2 ");
    #endif
    strip.setPixelColor(0, strip.Color(0, 128, 128));
  case 1:
    #ifdef DEBUG
    Serial.print("1 ");
    #endif
    strip.setPixelColor(56, strip.Color(0, 128, 128));
  case 0:
  default:
    break;
  }

  // Turn on the it, is
  #ifdef DEBUG
  Serial.print("IT ");
  #endif
  strip.setPixelColor(1, strip.Color(128, 128, 0));
  strip.setPixelColor(2, strip.Color(128, 128, 0));
  #ifdef DEBUG
  Serial.print("IS ");
  #endif
  strip.setPixelColor(4, strip.Color(128, 128, 0));
  strip.setPixelColor(5, strip.Color(128, 128, 0));

  // Set the minute
  i = now.Minute;
  if(i < 5) {
    //o'clock
    #ifdef DEBUG
    Serial.print("OCLOCK ");
    #endif
    strip.setPixelColor(57, strip.Color(0, 0, 255));
    strip.setPixelColor(58, strip.Color(0, 0, 255));
    strip.setPixelColor(59, strip.Color(0, 0, 255));
    strip.setPixelColor(60, strip.Color(0, 0, 255));
    strip.setPixelColor(61, strip.Color(0, 0, 255));
    strip.setPixelColor(62, strip.Color(0, 0, 255));
  } 
  else if(i < 10) {
    //five past
    #ifdef DEBUG
    Serial.print("FIVE MINUTES PAST ");
    #endif
    strip.setPixelColor(9, strip.Color(0, 0, 255));
    strip.setPixelColor(10, strip.Color(0, 0, 255));
    strip.setPixelColor(11, strip.Color(0, 0, 255));
    strip.setPixelColor(12, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 15) {
    //ten past
    #ifdef DEBUG
    Serial.print("TEN MINUTES PAST ");
    #endif
    strip.setPixelColor(13, strip.Color(0, 0, 255));
    strip.setPixelColor(14, strip.Color(0, 0, 255));
    strip.setPixelColor(15, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 20) {
    //quarter past
    #ifdef DEBUG
    Serial.print("A QUARTER PAST ");
    #endif
    strip.setPixelColor(69, strip.Color(0, 0, 255));
    
    strip.setPixelColor(72, strip.Color(0, 0, 255));
    strip.setPixelColor(73, strip.Color(0, 0, 255));
    strip.setPixelColor(74, strip.Color(0, 0, 255));
    strip.setPixelColor(75, strip.Color(0, 0, 255));
    strip.setPixelColor(76, strip.Color(0, 0, 255));
    strip.setPixelColor(77, strip.Color(0, 0, 255));
    strip.setPixelColor(78, strip.Color(0, 0, 255));

    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 25) {
    //twenty past
    #ifdef DEBUG
    Serial.print("TWENTY MINUTES PAST ");
    #endif
    strip.setPixelColor(7, strip.Color(0, 0, 255));
    strip.setPixelColor(64, strip.Color(0, 0, 255));
    strip.setPixelColor(65, strip.Color(0, 0, 255));
    strip.setPixelColor(66, strip.Color(0, 0, 255));
    strip.setPixelColor(67, strip.Color(0, 0, 255));
    strip.setPixelColor(68, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 30) {
    //twenty five past
    #ifdef DEBUG
    Serial.print("TWENTY FIVE MINUTES PAST ");
    #endif
    strip.setPixelColor(7, strip.Color(0, 0, 255));
    strip.setPixelColor(64, strip.Color(0, 0, 255));
    strip.setPixelColor(65, strip.Color(0, 0, 255));
    strip.setPixelColor(66, strip.Color(0, 0, 255));
    strip.setPixelColor(67, strip.Color(0, 0, 255));
    strip.setPixelColor(68, strip.Color(0, 0, 255));

    strip.setPixelColor(9, strip.Color(0, 0, 255));
    strip.setPixelColor(10, strip.Color(0, 0, 255));
    strip.setPixelColor(11, strip.Color(0, 0, 255));
    strip.setPixelColor(12, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 35) {
    //half past
    #ifdef DEBUG
    Serial.print("HALF PAST ");
    #endif
    strip.setPixelColor(17, strip.Color(0, 0, 255));
    strip.setPixelColor(18, strip.Color(0, 0, 255));
    strip.setPixelColor(19, strip.Color(0, 0, 255));
    strip.setPixelColor(20, strip.Color(0, 0, 255));

    strip.setPixelColor(25, strip.Color(0, 0, 255));
    strip.setPixelColor(26, strip.Color(0, 0, 255));
    strip.setPixelColor(27, strip.Color(0, 0, 255));
    strip.setPixelColor(28, strip.Color(0, 0, 255));
  } 
  else if(i < 40) {
    //twenty five to
    #ifdef DEBUG
    Serial.print("TWENTY FIVE MINUTES TO ");
    #endif
    strip.setPixelColor(7, strip.Color(0, 0, 255));
    strip.setPixelColor(64, strip.Color(0, 0, 255));
    strip.setPixelColor(65, strip.Color(0, 0, 255));
    strip.setPixelColor(66, strip.Color(0, 0, 255));
    strip.setPixelColor(67, strip.Color(0, 0, 255));
    strip.setPixelColor(68, strip.Color(0, 0, 255));

    strip.setPixelColor(9, strip.Color(0, 0, 255));
    strip.setPixelColor(10, strip.Color(0, 0, 255));
    strip.setPixelColor(11, strip.Color(0, 0, 255));
    strip.setPixelColor(12, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(85, strip.Color(0, 0, 255));
    strip.setPixelColor(86, strip.Color(0, 0, 255));
    thisHour++;
  } 
  else if(i < 45) {
    //twenty to
    #ifdef DEBUG
    Serial.print("TWENTY MINUTES TO ");
    #endif
    strip.setPixelColor(7, strip.Color(0, 0, 255));
    strip.setPixelColor(64, strip.Color(0, 0, 255));
    strip.setPixelColor(65, strip.Color(0, 0, 255));
    strip.setPixelColor(66, strip.Color(0, 0, 255));
    strip.setPixelColor(67, strip.Color(0, 0, 255));
    strip.setPixelColor(68, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(85, strip.Color(0, 0, 255));
    strip.setPixelColor(86, strip.Color(0, 0, 255));
    thisHour++;
  } 
  else if(i < 50) {
    //quarter to
    #ifdef DEBUG
    Serial.print("A QUARTER TO ");
    #endif
    strip.setPixelColor(69, strip.Color(0, 0, 255));
    
    strip.setPixelColor(72, strip.Color(0, 0, 255));
    strip.setPixelColor(73, strip.Color(0, 0, 255));
    strip.setPixelColor(74, strip.Color(0, 0, 255));
    strip.setPixelColor(75, strip.Color(0, 0, 255));
    strip.setPixelColor(76, strip.Color(0, 0, 255));
    strip.setPixelColor(77, strip.Color(0, 0, 255));
    strip.setPixelColor(78, strip.Color(0, 0, 255));

    strip.setPixelColor(85, strip.Color(0, 0, 255));
    strip.setPixelColor(86, strip.Color(0, 0, 255));
    thisHour++;
  } 
  else if(i < 55) {
    //ten to
    #ifdef DEBUG
    Serial.print("TEN MINUTES TO ");
    #endif
    strip.setPixelColor(13, strip.Color(0, 0, 255));
    strip.setPixelColor(14, strip.Color(0, 0, 255));
    strip.setPixelColor(15, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(85, strip.Color(0, 0, 255));
    strip.setPixelColor(86, strip.Color(0, 0, 255));
    thisHour++;
  } 
  else {
    //five to
    #ifdef DEBUG
    Serial.print("FIVE MINUTES TO ");
    #endif
    strip.setPixelColor(9, strip.Color(0, 0, 255));
    strip.setPixelColor(10, strip.Color(0, 0, 255));
    strip.setPixelColor(11, strip.Color(0, 0, 255));
    strip.setPixelColor(12, strip.Color(0, 0, 255));

    strip.setPixelColor(21, strip.Color(0, 0, 255));
    strip.setPixelColor(22, strip.Color(0, 0, 255));
    strip.setPixelColor(23, strip.Color(0, 0, 255));
    strip.setPixelColor(80, strip.Color(0, 0, 255));
    strip.setPixelColor(81, strip.Color(0, 0, 255));
    strip.setPixelColor(82, strip.Color(0, 0, 255));
    strip.setPixelColor(83, strip.Color(0, 0, 255));
    
    strip.setPixelColor(85, strip.Color(0, 0, 255));
    strip.setPixelColor(86, strip.Color(0, 0, 255));
    thisHour++;
  }

  if(thisHour > 24) {
    amPm = 0;
    thisHour = 1;
  } 
  else if(thisHour == 24) {
    thisHour = 12;
    amPm = 0;
  } 
  else if(thisHour > 12) {
    thisHour -= 12;
    amPm = 1;
  } 
  else if(thisHour == 12) {
    thisHour = 12;
    amPm = 1;
  } 
  else {
    amPm = 0;
  }

  // Set the hour
  i = thisHour;
  if(i > 12) i -= 12;
  if(i == 0) i = 12;
  switch(i) {
  case 12:
    #ifdef DEBUG
    Serial.print("TWELVE ");
    #endif
    strip.setPixelColor(30, strip.Color(0, 255, 0));
    strip.setPixelColor(31, strip.Color(0, 255, 0));
    strip.setPixelColor(88, strip.Color(0, 255, 0));
    strip.setPixelColor(89, strip.Color(0, 255, 0));
    strip.setPixelColor(90, strip.Color(0, 255, 0));
    strip.setPixelColor(91, strip.Color(0, 255, 0));
    break;
  case 11:
    #ifdef DEBUG
    Serial.print("ELEVEN ");
    #endif
    strip.setPixelColor(97, strip.Color(0, 255, 0));
    strip.setPixelColor(98, strip.Color(0, 255, 0));
    strip.setPixelColor(99, strip.Color(0, 255, 0));
    strip.setPixelColor(100, strip.Color(0, 255, 0));
    strip.setPixelColor(101, strip.Color(0, 255, 0));
    strip.setPixelColor(102, strip.Color(0, 255, 0));
    break;
  case 10:
    #ifdef DEBUG
    Serial.print("TEN ");
    #endif
    strip.setPixelColor(104, strip.Color(0, 255, 0));
    strip.setPixelColor(105, strip.Color(0, 255, 0));
    strip.setPixelColor(106, strip.Color(0, 255, 0));
    break;
  case 9:
    #ifdef DEBUG
    Serial.print("NINE ");
    #endif
    strip.setPixelColor(115, strip.Color(0, 255, 0));
    strip.setPixelColor(116, strip.Color(0, 255, 0));
    strip.setPixelColor(117, strip.Color(0, 255, 0));
    strip.setPixelColor(118, strip.Color(0, 255, 0));
    break;
  case 8:
    #ifdef DEBUG
    Serial.print("EIGHT ");
    #endif
    strip.setPixelColor(54, strip.Color(0, 255, 0));
    strip.setPixelColor(55, strip.Color(0, 255, 0));
    strip.setPixelColor(112, strip.Color(0, 255, 0));
    strip.setPixelColor(113, strip.Color(0, 255, 0));
    strip.setPixelColor(114, strip.Color(0, 255, 0));
    break;
  case 7:
    #ifdef DEBUG
    Serial.print("SEVEN ");
    #endif
    strip.setPixelColor(49, strip.Color(0, 255, 0));
    strip.setPixelColor(50, strip.Color(0, 255, 0));
    strip.setPixelColor(51, strip.Color(0, 255, 0));
    strip.setPixelColor(52, strip.Color(0, 255, 0));
    strip.setPixelColor(53, strip.Color(0, 255, 0));
    break;
  case 6:
    #ifdef DEBUG
    Serial.print("SIX ");
    #endif
    strip.setPixelColor(33, strip.Color(0, 255, 0));
    strip.setPixelColor(34, strip.Color(0, 255, 0));
    strip.setPixelColor(35, strip.Color(0, 255, 0));
    break;
  case 5:
    #ifdef DEBUG
    Serial.print("FIVE ");
    #endif
    strip.setPixelColor(44, strip.Color(0, 255, 0));
    strip.setPixelColor(45, strip.Color(0, 255, 0));
    strip.setPixelColor(46, strip.Color(0, 255, 0));
    strip.setPixelColor(47, strip.Color(0, 255, 0));
    break;
  case 4:
    #ifdef DEBUG
    Serial.print("FOUR ");
    #endif
    strip.setPixelColor(107, strip.Color(0, 255, 0));
    strip.setPixelColor(108, strip.Color(0, 255, 0));
    strip.setPixelColor(109, strip.Color(0, 255, 0));
    strip.setPixelColor(110, strip.Color(0, 255, 0));
    break;
  case 3:
    #ifdef DEBUG
    Serial.print("THREE ");
    #endif
    strip.setPixelColor(36, strip.Color(0, 255, 0));
    strip.setPixelColor(37, strip.Color(0, 255, 0));
    strip.setPixelColor(38, strip.Color(0, 255, 0));
    strip.setPixelColor(39, strip.Color(0, 255, 0));
    strip.setPixelColor(96, strip.Color(0, 255, 0));
    break;
  case 2:
    #ifdef DEBUG
    Serial.print("TWO ");
    #endif
    strip.setPixelColor(41, strip.Color(0, 255, 0));
    strip.setPixelColor(42, strip.Color(0, 255, 0));
    strip.setPixelColor(43, strip.Color(0, 255, 0));
    break;
  case 1:
    #ifdef DEBUG
    Serial.print("ONE ");
    #endif
    strip.setPixelColor(92, strip.Color(0, 255, 0));
    strip.setPixelColor(93, strip.Color(0, 255, 0));
    strip.setPixelColor(94, strip.Color(0, 255, 0));
    break;
  default:
    break;
  }

  // Set AM / PM
  if(!amPm) {
    #ifdef DEBUG
    Serial.print("AM ");
    #endif
    strip.setPixelColor(120, strip.Color(255, 0, 0));
    strip.setPixelColor(121, strip.Color(255, 0, 0));
  } 
  else {
    #ifdef DEBUG
    Serial.print("PM ");
    #endif
    strip.setPixelColor(122, strip.Color(255, 0, 0));
    strip.setPixelColor(123, strip.Color(255, 0, 0));
  }

  #ifdef DEBUG
  Serial.println();
  #endif
  
  strip.show();
}
