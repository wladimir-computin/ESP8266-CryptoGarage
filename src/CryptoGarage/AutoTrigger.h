#ifndef AUTOTRIGGER_H
#define AUTOTRIGGER_H

#include <Ticker.h>

#include "Debug.h"

#define ENABLE_STATUS_LED 1

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
