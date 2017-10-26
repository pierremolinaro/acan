//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// This sketch demonstrates how send and receive remote frames using the ACAN library:
//   - Can0 sends remote frames (Can 0 uses alternate Rx and Tx pins)
//   - Can1 receives remote frames
// Serial is used for displaying sent and received frame count.
// This sketch needs to establish a CAN network that connects CAN0 and CAN1.
// You can use a single AND gate, as 74HC08, powered on 3.3V:
//   - AND inputs are CANT0X and CAN1TX;
//   - AND outputs are CAN0RX and CAN1RX.
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ACAN.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __MK66FX1M0__
  #error "This sketch should be compiled for Teensy 3.6"
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void setup () {
  Serial.begin (9600) ;
//--- ACAN::can1 receives remote frames
  ACANPrimaryFilter filter [1] = {
    ACANPrimaryFilter (kRemote, kStandard, 0x542) // Receive remote standard frames, with id=0x542
  } ;
  ACANSettings settings (250 * 1000) ; // 250 kbit/s
  uint32_t r = ACAN::can1.begin (settings, filter, 1) ;
  Serial.print ("Can1 configuration: 0x") ;
  Serial.println (r, HEX) ;
  settings.mUseAlternateRxPin = true ; // Comment out for not using alternate Rx pin for Can0
  settings.mUseAlternateTxPin = true ; // Comment out for not using alternate Tx pin for Can0
//--- ACAN::can0 sends remote frames
  r = ACAN::can0.begin (settings) ;
  Serial.print ("Can0 configuration: 0x") ;
  Serial.println (r, HEX) ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static unsigned gSendDeadline = 0 ;
static unsigned gSentCount = 0 ;
static unsigned gReceivedCount = 0 ;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void loop () {
  CANMessage frame ;
  if (gSendDeadline < millis ()) { 
    frame.id = 0x542 ;
    frame.rtr = true ;
    const bool ok = ACAN::can0.tryToSend (frame) ;
    if (ok) {
      gSendDeadline += 2000 ; // Send next frame 2 seconds later
      gSentCount += 1 ;
      Serial.print ("Sent: ") ;
      Serial.println (gSentCount) ;
    }
  }
  if (ACAN::can1.receive (frame)) {
    gReceivedCount += 1 ;
    Serial.print ("Received: ") ;
    Serial.println (gReceivedCount) ;
  }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

