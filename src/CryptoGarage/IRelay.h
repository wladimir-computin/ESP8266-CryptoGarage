/*
* CryptoGarage - IRelay
* Abstract class for controlling relays.
*/


#ifndef IRELAY_H
#define IRELAY_H

#include <Arduino.h>
#include <Ticker.h>

#include "Debug.h"


class IRelay{
  protected:
    Ticker relayTicker;
    static void relayTickerTick(void * context);
    
  public:
    virtual void setState(bool state) = 0;
    virtual bool getState() = 0;
    void trigger();
};

#endif
