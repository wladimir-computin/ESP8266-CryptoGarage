/*
* CryptoGarage - RateLimit
* 
* (implementation)
*/

#include "RateLimit.h"

void RateLimit::rateLimitTick(void * context) {
  (*(RateLimit*)context).setState(OPEN);
}

void RateLimit::setState(RateLimitState state) {
  switch (state) {
    case OPEN:
      //printDebug("RateLimit: OPEN");
      rateLimitState = OPEN;
      break;
    case BLOCKED:
      if (rateLimitState != BLOCKED) {
        //printDebug("RateLimit: BLOCKED");
        if(RATE_LIMIT_TIMEOUT_MS != 0){
          rateLimitTicker.once_ms(RATE_LIMIT_TIMEOUT_MS, rateLimitTick, (void*)this);
          rateLimitState = BLOCKED;
        }
      }
      break;
  }
}

RateLimitState RateLimit::getState(){
  return rateLimitState;
}

