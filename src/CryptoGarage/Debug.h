/*
* CryptoGarage - Debug
* 
* Debugging macro.
* If DEBUG is set to 1, printDebug() calls in this project get translated to Serial.println(). Otherwise they will be ignored by the compiler.
* (At least I hope while(false) will be ignored by the compiler, otherwise the compiler is dump)
*/

#ifndef DEBUG_H
#define DEBUG_H

  #include <Arduino.h>
  #include "AllConfig.h"
  
  #if DEBUG == 1 //Defined in AllConfig.h
    #define printDebug(x) Serial.println(x)
    #define printfDebug(x...) Serial.printf(x)
  #else
    #define printDebug(x) while(false)
    #define printfDebug(x...) while(false)
  #endif
  
#endif
