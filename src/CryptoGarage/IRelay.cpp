/*
* CryptoGarage - IRelay
* 
* (implementation)
*/

#include "AllConfig.h"
#include "IRelay.h"


void IRelay::relayTickerTick(void * context){
  (*(IRelay*)context).setState(false);
}

void IRelay::trigger(){
  setState(true);
  relayTicker.once_ms(RELAY_TRIGGER_TIME_MS, relayTickerTick, (void*)this); //call setState(false) after [RELAY_TRIGGER_TIME_MS] milliseconds passed.
}

