/*
* CryptoGarage - AutoTrigger
* 
* The main idea of this class is that the garage gate closes automatically after
* I leave it and go out of WiFi range. It works like a watchdog, constantly counting
* backward until 0 and then triggering the relay. A ping resets the counter, so the
* gate keeps open as long as I'm in range.
* 
* However, this doesn't work 100%, as it may lose the connection and the gate starts
* to close while I'm still inside.
* Use with caution or develop something better!
*/

#ifndef AUTOTRIGGER_H
#define AUTOTRIGGER_H

#include <Ticker.h>

#include "AllConfig.h"
#include "Debug.h"

#if ENABLE_STATUS_LED == 1
  #include "StatusLED.h"
#endif

class AutoTrigger {
  
  private:
    static void AutoTriggerTick(void * context);
    void AutoTriggerTickStuff();
    Ticker AutoTriggerTicker;
    bool active = false;
    bool finished = false;
    int tickerCount = 0;
    int tickerGoal = 10;
    #if ENABLE_STATUS_LED == 1
      StatusLED &led = StatusLED::instance();
    #endif
      
  public:
      void engage();
      void disengage();
      bool isActive();
      bool isFinished();
      void setGoal(int goal);
      void resetCounter();
};

#endif
