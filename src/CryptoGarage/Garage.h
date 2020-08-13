/*
* CryptoGarage - Garage
* 
* TODO
*/

#ifndef GARAGE_H
#define GARAGE_H

#include "Device.h"
#include "PersistentMemory.h"
#include "AutoTrigger.h"
#ifdef GARAGE_GATE_STATUS_PIN
  #include "GarageGate2.h"
#else
  #include "GarageGate.h"
#endif

#if RELAYLCTECH == 1
  #include "LCTechRelay.h"
#elif RELAYWEMOS == 1
  #include "WemosShieldRelay.h"
//#elif
  //your relay here...
#endif

const char DEVICENAME[] = "Garage";

const char KEY_AUTOTRIGGER_TIMEOUT[] = "autotrigger_timeout";

const char COMMAND_TRIGGER[] = "trigger";
const char COMMAND_TRIGGER_AUTO[] = "autotrigger";
const char COMMAND_AUTOTRIGGER_PING[] = "ping";
const char COMMAND_GATESTATE[] = "gatestate";

class Garage : public Device {
  public:
    //Here we process the plaintext commands and generate an answer for the client.
    ProcessMessageStruct processMessage(String &message);
    String getName();
    String getStatus();
    void loop();
    void setup();
    
  private:

    #if RELAYLCTECH == 1
      LCTechRelay relay;  
    #elif RELAYWEMOS == 1
      WemosShieldRelay relay;
    //#elif
      //your relay here...
    #endif

    //Garage specific stuff
    AutoTrigger autoTrigger;
    GarageGate gateState;
    
    void triggerRelay();
};

#endif
