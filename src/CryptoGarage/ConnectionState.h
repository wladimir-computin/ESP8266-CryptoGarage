/*
* CryptoGarage - ConnectionState
* 
* Class for tracking the state of the challenge-response protocol.
* The state jumps back to NONE after CONNECTION_STATE_TIMEOUT seconds (defined in AllConfig.h)
* This makes replay attacks or brute forcing the 12 byte challenge IV practically impossible.
*/

#ifndef CONNECTIONSTATE_H
#define CONNECTIONSTATE_H

#include <Ticker.h>

#include "AllConfig.h"
#include "Debug.h"

enum ConState {NONE, PHASE2};

class ConnectionState {
  private:
    Ticker conStateTicker;
    static void conStateTick(void * context);
    ConState conState = NONE;

  public:
    void setState(ConState con);
    ConState getState();
  
};

#endif
