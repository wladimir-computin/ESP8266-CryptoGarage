/*
* CryptoGarage - Time
* 
* TODO
*/

#ifndef TIME_H
#define TIME_H

#include "AllConfig.h"
#include "Arduino.h"
#include "PersistentMemory.h"
#include <time.h>
#include <sunset.h>
#include "Debug.h"

const char SERVER1[] = "fritz.box";
const char SERVER2[] = "pool.ntp.org";
const char TZENV[] = "CET-1CEST,M3.5.0,M10.5.0/3";
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

const char KEY_CUSTOM_SUNRISE_ANGLE[] = "custom_sunrise_angle";
const char KEY_SERVER1[] = "ntp_server1";
const char KEY_SERVER2[] = "ntp_server2";
const char KEY_TZENV[] = "tz_env";
  

class Time {
  private:
    Time() {}           
    Time( const Time& );
    Time & operator = (const Time &);

    uint8_t custom_sunrise_angle = 93;
    double position_lat = 52.3085367;
    double position_long = 9.5881486;

    int old_day = -1;

    SunSet getSunSet();
  
  public:
    static Time& instance() {
      static Time _instance;
      return _instance;
    }
    ~Time() {}

    void setup();
    tm * now(bool utc=false);
    int minutesSinceMidnight(bool utc=false);
    String stringTime(bool utc=false);
    int sunrise_minutes();
    int sunset_minutes();
    int civil_sunrise_minutes();
    int civil_sunset_minutes();
    int custom_sunrise_minutes(double degree=93);
    int custom_sunset_minutes(double degree=93);
    static String min2str(int minutes);
    int timezone();
    bool isNight();
};

#endif 
