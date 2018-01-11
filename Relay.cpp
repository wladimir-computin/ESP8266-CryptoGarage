/*
* CryptoGarage - Relay
* (implementation)
*/

#include "Relay.h"

const int TRIGGER_TIME_MS = 250;

void Relay::relayTickerTick(void * context){
	(*(Relay*)context).setState(false);
}

void Relay::setState(bool state){
	relayTicker.detach();
  if(state == true){
    Serial.write(RELAY_ON, sizeof(RELAY_ON));
    printDebug("Relay up");
  } else {
    Serial.write(RELAY_OFF, sizeof(RELAY_OFF));
    printDebug("Relay down");
  }
  Serial.flush();
  relayState = state;
}

bool Relay::getState(){
  return relayState;
}

void Relay::trigger(){
	setState(true);
	relayTicker.attach_ms(TRIGGER_TIME_MS, relayTickerTick, (void*)this); //call setState(false) after [TRIGGER_TIME_MS] milliseconds passed.
}

