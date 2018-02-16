/*
* CryptoGarage - GarageGate
* 
* (implementation)
*/

#include "GarageGate.h"

void GarageGate::gateTickerTick(void * context) {
  (*(GarageGate*)context).gateTickerTickStuff();
}

void GarageGate::gateTickerTickStuff(){
  gateTicker.detach();
  gateState = STILL;
}

state GarageGate::getState(){
  return gateState;
}

void GarageGate::trigger(){
  gateTicker.detach();
  if (gateState == MOVING){
    gateState = STILL;
  } else {
    gateState = MOVING;
    gateTicker.attach(movingTimeSeconds, gateTickerTick, (void*)this);
  }
}

