/*
* CryptoGarage - ConnectionState
* 
* (implementation)
*/

#include "ConnectionState.h"

void ConnectionState::conStateTick(void * context) {
  (*(ConnectionState*)context).setState(NONE);
}

void ConnectionState::setState(ConState con) {
  switch (con) {
    case NONE:
      printDebug("ConnectionState: NONE");
      conState = NONE;
      break;
    case PHASE2:
      printDebug("ConnectionState: PHASE2");
      if (conState != PHASE2) {
        conStateTicker.once(CONNECTION_STATE_TIMEOUT, conStateTick, (void*)this);
        conState = PHASE2;
      }
      break;
  }
}

ConState ConnectionState::getState(){
  return conState;
}

