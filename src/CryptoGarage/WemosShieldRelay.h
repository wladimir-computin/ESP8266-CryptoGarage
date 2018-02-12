/*
* CryptoGarage - WemosShieldRelay
* Class for controlling the relay shield of a Wemos D1 Mini (Pro)
*/
#ifndef WEMOSSHIELDRELAY_H
#define WEMOSSHIELDRELAY_H

#include "IRelay.h"


class WemosShieldRelay: public IRelay{
  private:
    bool relayState = false;
    const int relayPin = 5; //D1
    
  public:
    WemosShieldRelay();
    void setState(bool state);
    bool getState();
};

#endif
