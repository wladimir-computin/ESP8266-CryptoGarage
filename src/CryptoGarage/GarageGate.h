#ifndef GARAGEGATE_H
#define GARAGEGATE_H

#include <Ticker.h>

#include "Debug.h"

//enum state {CLOSED, OPENING, OPEN, CLOSING, STOPPED_OPENING, STOPPED_CLOSING}; //later...
enum state {STILL, MOVING};

class GarageGate {
  private:
    const int movingTimeSeconds = 10;
    Ticker gateTicker;
    static void gateTickerTick(void * context);
    void gateTickerTickStuff();
    state gateState = STILL;

  public:
    state getState();
    void setState();
    void trigger();
  
};

#endif

