#include "ConnectionState.h"

void ConnectionState::conStateTick(void * context) {
  (*(ConnectionState*)context).setState(NONE);
}

void ConnectionState::setState(ConState con) {
  switch (con) {
    case NONE:
      printDebug("ConnectionState: NONE");
      conStateTicker.detach();
      conState = NONE;
      break;
    case PHASE2:
      printDebug("ConnectionState: PHASE2");
      if (conState != PHASE2) {
        conStateTicker.attach(2, conStateTick, (void*)this);
        conState = PHASE2;
      }
      break;
  }
}

ConState ConnectionState::getState(){
  return conState;
}

