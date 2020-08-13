/*
* CryptoGarage - Uptime
* 
* This class measures the time since the last reboot.
*/

#ifndef UPTIME_H
#define UPTIME_H

#include <Ticker.h>

#include "AllConfig.h"
#include "Debug.h"

class Uptime {
  private:
    Uptime() {}           
    Uptime( const Uptime& );
    Uptime & operator = (const Uptime &);
    
    Ticker uptimeTicker;
    static void uptimeTick(void * context);
    uint32_t secsUp;

  public:
    static Uptime& instance() {
      static Uptime _instance;
      return _instance;
    }
    ~Uptime() {}
    
    void start();
    
    String getUptime();
};

#endif
