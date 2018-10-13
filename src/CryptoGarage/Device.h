/*
* CryptoGarage - Device
* 
* TODO
*/

#ifndef DEVICE_H
#define DEVICE_H

#include "AllConfig.h"
#include "Debug.h"
#include "Message.h"

class Device{
  public:
    virtual ProcessMessageStruct processMessage(String &message) = 0;
    virtual String getStatus() = 0;
    virtual void loop(){};
    virtual void setup(){};
};

#endif
