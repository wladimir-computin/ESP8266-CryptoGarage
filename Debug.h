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

  #define DEBUG 0 //Change to 1 for debug output
  
  #if DEBUG == 1
    #define printDebug(x) Serial.println(x)
  #else
    #define printDebug(x) while(false)
  #endif
  
#endif
