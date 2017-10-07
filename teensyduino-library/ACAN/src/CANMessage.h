//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ACAN: CAN Library for Teensy 3.1 / 3.2, 3.5, 3.6
// https://github.com/pierremolinaro/acan
// Generic CAN Message
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __CAN_MESSAGE_H__
#define __CAN_MESSAGE_H__

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Arduino.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class CANMessage {
  public : uint32_t id = 0 ;  // Frame identifier
  public : bool ext = false ; // false -> standard frame, true -> extended
  public : bool rtr = false ; // false -> data frame, true -> remote frame
  public : uint8_t idx = 0 ;
  public : uint8_t len = 0 ; // Length of data
  public : union {
    #ifdef __UINT64_TYPE__
      uint64_t data64   ; // Caution: subject to endianness
    #endif
    uint32_t data32 [2] ; // Caution: subject to endianness
    uint16_t data16 [4] ; // Caution: subject to endianness
    uint8_t  data   [8] = {0, 0, 0, 0, 0, 0, 0, 0} ;
  } ;
} ;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif // __CAN_MESSAGE_H__
