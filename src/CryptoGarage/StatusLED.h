/*
  CryptoGarage - StausLED
  
  Class for controlling the builtin LED
*/

#ifndef StatusLED_H
#define StatusLED_H

#include <Arduino.h>
#include <Ticker.h>

#include "Debug.h"


class StatusLED {
  
  public:
    enum FadeMode {NONE, SINGLE_ON_OFF, SINGLE_OFF_ON, PERIODIC_FADE};


  private:
    StatusLED() {}           
    StatusLED( const StatusLED& );
    StatusLED & operator = (const StatusLED &);

    struct Argument {
      int time_ms;
      int old_current;
      int goal;
      int ms_passed;
      FadeMode fademode;
    };

    Ticker ledTicker;
    int current = 0;
    const int ledPin = BUILTIN_LED;
    Argument argument = {0,0,0,0,NONE};
    static void ledTickerTick(void * callback);
    void ledTickerTick2();
    void fadeCompleted();
    int conv1023(int val);
    const int MAXLED = 1;
    const int MINLED = 1023;


  public:
    static StatusLED& instance() {
      static StatusLED _instance;
      return _instance;
    }
    ~StatusLED() {}

    void setState(bool state);
    void setVal(int val);
    void fade(FadeMode fademode, int time_ms);
    void fadeToVal(int val, int time_ms);
    void fadeToVal(int val, int time_ms, void (*callback)());
    void fadeStop();

};

#endif
