/*
* CryptoGarage - IRelay
* (implementation)
*/

#include "IRelay.h"

const int TRIGGER_TIME_MS = 250;

void IRelay::relayTickerTick(void * context){
  (*(IRelay*)context).setState(false);
}

void IRelay::trigger(){
  setState(true);
  relayTicker.once_ms(TRIGGER_TIME_MS, relayTickerTick, (void*)this); //call setState(false) after [TRIGGER_TIME_MS] milliseconds passed.
}

