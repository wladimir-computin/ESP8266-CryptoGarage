/*
* CryptoGarage - WemosShieldRelay
* 
* (implementation)
*/

#include "WemosShieldRelay.h"

WemosShieldRelay::WemosShieldRelay(){
  pinMode(relayPin, OUTPUT);
}

void WemosShieldRelay::setState(bool state){
  digitalWrite(relayPin, state);
  relayState = state;
}

bool WemosShieldRelay::getState(){
  return relayState;
}

