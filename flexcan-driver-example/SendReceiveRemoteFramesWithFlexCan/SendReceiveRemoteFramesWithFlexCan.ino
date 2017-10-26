//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// This sketch shows that FlexCan connot send properly remote frames, and cannot receive them):
//   - Can0 is programmed for sending remote frames (Can 0 uses alternate Rx and Tx pins)
//   - Can1 is programmed for receiving remote frames
// Serial is used for displaying sent and received frame count
//   - no remote frame is received;
//   - 17 remote frames are "sent"; in fact, only the first one is actually sent. The 16 next frames are only buffered
//     in the driver send buffer.
// This sketch needs to establish a CAN network that connects CAN0 and CAN1.
// You can use a single AND gate, as 74HC08, powered on 3.3V:
//   - AND inputs are CANT0X and CAN1TX;
//   - AND outputs are CAN0RX and CAN1RX.
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <FlexCAN.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __MK66FX1M0__
  #error "This sketch should be compiled for Teensy 3.6"
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void setup() {
  Serial.begin (9600) ;
//--- Configure Can0
  CAN_filter_t allPassFilter ;
  allPassFilter.id = 0 ;
  allPassFilter.ext = 0 ;
  allPassFilter.rtr = 0 ;
  Can0.begin (125 * 1000, allPassFilter, 1, 1) ; // Alternate Rx and Tx pins are used
//--- Configure Can1
  Can1.begin (125 * 1000) ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static unsigned gSendDeadline = 0 ;
static unsigned gSentCount = 0 ;
static unsigned gReceivedCount = 0 ;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void loop() {
  CAN_message_t frame ;
  if (gSendDeadline < millis ()) {
    frame.id = 0x542 ;
    frame.rtr = true ;
    frame.ext = false ;
    frame.len = 0 ;
    const bool ok = Can0.write (frame) ; // Send next frame 2 seconds later
    if (ok) {
      gSendDeadline += 2000 ;
      gSentCount += 1 ;
      Serial.print ("Sent: ") ;
      Serial.println (gSentCount) ;
    }
  }
  if (Can1.read (frame)) {
    gReceivedCount += 1 ;
    Serial.print ("Received: ") ;
    Serial.println (gReceivedCount) ;
  }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

