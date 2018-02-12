#ifndef CONNECTIONSTATE_H
#define CONNECTIONSTATE_H

#include <Ticker.h>

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
