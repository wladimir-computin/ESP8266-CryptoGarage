/*
* CryptoGarage - RateLimit
* 
* The ESP8266 chip may get stuck and keep hanging if an user or attacker sends many request a little too quickly.
* Therefore I implemented a simple rate limit which works like a simple state machine.
* After setState(BLOCKED) is called, the internal state falls back to OPEN after RATE_LIMIT_TIMEOUT_MS (defined in AllConfig.h)
* Rate limiting is enabled for both, complete (e.g. trigger relay) and corrupted (e.g. wrong password etc.) requests.
* Set RATE_LIMIT_TIMEOUT_MS to 0 to disable rate limiting. For some commands like set settings rate limit is disabled by default (see CryptoGarage.ino)
*/

#ifndef RATELIMIT_H
#define RATELIMIT_H

#include <Ticker.h>

#include "AllConfig.h"
#include "Debug.h"

enum RateLimitState {OPEN, BLOCKED};

class RateLimit {
  private:
    Ticker rateLimitTicker;
    static void rateLimitTick(void * context);
    RateLimitState rateLimitState = OPEN;

  public:
    void setState(RateLimitState con);
    RateLimitState getState();
  
};

#endif
