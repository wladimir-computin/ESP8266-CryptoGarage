/*
* Prog2Morse - Relay
* Class for controlling the LC-Tech Relay shield for ESP-01(S).
* This relay is not controlled by a digital pin, but instead it has it's own serial chip which listens on 9600 baud for a specific byte code sequence.
* (Chinese documentation really sucks)
*/


#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include <Ticker.h>

#include "Debug.h"

const byte RELAY_ON[] = {0xA0, 0x01, 0x01, 0xA2};
const byte RELAY_OFF[] = {0xA0, 0x01, 0x00, 0xA1};

class Relay{
  private:
    bool relayState = false;
    Ticker relayTicker;
    static void relayTickerTick(void * context);
    
  public:
    void setState(bool state);
    bool getState();
    void trigger();
};

#endif
