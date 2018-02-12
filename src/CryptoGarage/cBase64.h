#ifndef _CBASE64_H
#define _CBASE64_H

#include <Arduino.h>

class cBase64{
  public:
    int encode(uint8_t * output, uint8_t * input, int inputLength);
    int decode(uint8_t * output, char * input, int inputLength);
    int encodedLength(int plainLength);
    int decodedLength(char * input, int inputLength);

  private:
    inline void fromA3ToA4(unsigned char * A4, unsigned char * A3);
    inline void fromA4ToA3(unsigned char * A3, unsigned char * A4);
    inline unsigned char lookupTable(char c);
};
extern cBase64 base64;

#endif
