// LoopBackDemo

// This demo runs on Teensy 3.1 / 3.2, 3.5 and 3.6
// The FlexCAN module is configured in loop back mode: no external hardware required. 

//-----------------------------------------------------------------

#include <ACAN.h>

//-----------------------------------------------------------------

void setup () {
  Serial.begin (9600) ;
  Serial.println ("Hello") ;
  ACANSettings settings (125 * 1000) ; // 125 kbit/s
  settings.mLoopBackMode = true ; // Loop back mode
  settings.mSelfReceptionMode = true ; // Required for loop back mode
  const ACANPrimaryFilter primaryFilters [] = {
    ACANPrimaryFilter (kData, kStandard, 0x542, handleMessage_0),
    ACANPrimaryFilter (kRemote, kExtended, 0x12345678, handleMessage_1)
  } ;
  const uint32_t errorCode = ACAN::can0.begin (settings, primaryFilters, 2) ;
  if (0 == errorCode) {
    Serial.println ("can0 ok") ;
  }else{
    Serial.print ("Error can0: 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

//-----------------------------------------------------------------

static unsigned gSendDate0 = 0 ;
static unsigned gReceptionCount0 = 0 ;
static unsigned gSendCount0 = 0 ;

static unsigned gSendDate1 = 0 ;
static unsigned gReceptionCount1 = 0 ;
static unsigned gSendCount1 = 0 ;

//-----------------------------------------------------------------

static void handleMessage_0 (const CANMessage & inMessage) {
  gReceptionCount0 ++ ;
  Serial.print ("Received standard data 0x542: ") ; 
  Serial.println (gReceptionCount0) ; 
}

//-----------------------------------------------------------------

static void handleMessage_1 (const CANMessage & inMessage) {
  gReceptionCount1 ++ ;
  Serial.print ("Received extended remote 0x12345678: ") ; 
  Serial.println (gReceptionCount1) ; 
}

//-----------------------------------------------------------------

void loop () {
  CANMessage message ;
//--- Send message 0
  if (gSendDate0 < millis ()) {
    message.id = 0x12345678 ;
    message.rtr = true ;
    message.ext = true ;
    const bool ok = ACAN::can0.tryToSend (message) ;
    if (ok) {
      gSendDate0 += 2000 ;
      gSendCount0 ++ ;
      Serial.print ("Send standard data 0x542: ") ; 
      Serial.println (gSendCount0) ; 
    }
  }
//--- Send message 1
  if (gSendDate1 < millis ()) {
    message.id = 0x12345678 ;
    message.rtr = true ;
    message.ext = true ;
    const bool ok = ACAN::can0.tryToSend (message) ;
    if (ok) {
      gSendDate1 += 2500 ;
      gSendCount1 ++ ;
      Serial.print ("Send extended remote 0x12345678: ") ; 
      Serial.println (gSendCount1) ; 
    }
  }
//--- Received message handling
  ACAN::can0.dispatchReceivedMessage () ;
}
