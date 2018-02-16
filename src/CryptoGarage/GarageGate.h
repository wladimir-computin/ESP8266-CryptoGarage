/*
* CryptoGarage - GarageGate
* 
* Class for tracking the state of the garage gate. For now it just differentiates between STILL (e.g. open or closed) or MOVING (you know, moving)
* Works like a state machine, after calling trigger() the gate is in MOVING state for n seconds or util trigger() is called again.
* setState() not implemented (because not needed) yet.
*/

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

