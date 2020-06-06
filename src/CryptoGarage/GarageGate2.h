/*
* CryptoGarage - GarageGate2
* 
* Class for tracking the state of the garage gate. For now it just differentiates between STILL (e.g. open or closed) or MOVING (you know, moving)
* Works like a state machine, after calling trigger() the gate is in MOVING state for n seconds or util trigger() is called again.
* setState() not implemented (because not needed) yet.
*/

#ifndef GARAGEGATE2_H
#define GARAGEGATE2_H

#include <Ticker.h>

#include "Debug.h"

enum state {GATE_NONE, GATE_CLOSED, GATE_OPENING, GATE_OPEN, GATE_CLOSING, GATE_STOPPED_OPENING, GATE_STOPPED_CLOSING};

class GarageGate {
  private:
    const int movingTimeSeconds = 10;
    Ticker gateTicker;
    static void gateTickerTick(void * context);
    void gateTickerTickStuff();
    void updateState(int read);
	
    state gateState = GATE_NONE;
    int lastRead = -1;

  public:
    state getState();
    String getStateStr();
    void setState();
    void trigger();
    void loop();
    void setup();
};

#endif
