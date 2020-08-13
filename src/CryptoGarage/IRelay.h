/*
* CryptoGarage - IRelay
* 
* Abstract class for controlling relays.
* If you want to add your own relay, you have to implement the setState() and getState() methods.
* Take a look at LCTechRelay and WemosShieldRelay for examples.
*/

#ifndef IRELAY_H
#define IRELAY_H

#include <Arduino.h>
#include <Ticker.h>

#include "Debug.h"

const int RELAY_TRIGGER_TIME_MS = 250;  //click ...time in ms... clack

class IRelay{
  protected:
    Ticker relayTicker;
    static void relayTickerTick(void * context);
    
  public:
    //pass true to close (e.g. light on) relay and false to open  it (e.g. light off)
    virtual void setState(bool state) = 0;
    virtual bool getState() = 0;
    //setState(true), delay (via Timer), setState(false)
    void trigger();
};

#endif
