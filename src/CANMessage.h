//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Generic CAN Message
// by Pierre Molinaro & Jean-Luc Béchennec
// https://github.com/pierremolinaro/acan
//
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#pragma once

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Arduino.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class CANMessage {
  public : uint32_t id = 0 ;  // Frame identifier
  public : bool ext = false ; // false -> standard frame, true -> extended frame
  public : bool rtr = false ; // false -> data frame, true -> remote frame
  public : uint8_t idx = 0 ;  // This field is used by the ACAN driver
  public : uint8_t len = 0 ;  // Length of data (0 ... 8)
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
