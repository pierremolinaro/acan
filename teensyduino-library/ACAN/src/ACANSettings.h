//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ACAN: CAN Library for Teensy 3.1 / 3.2, 3.5, 3.6
// https://github.com/pierremolinaro/acan
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __ACAN_SETTINGS_H__
#define __ACAN_SETTINGS_H__

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdint.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class ACANSettings {
//--- Constructor for a given baud rate
  public : explicit ACANSettings (const uint32_t inWhishedBitRate,
                                  const uint32_t inTolerancePPM = 1000) ;

//--- CAN bit timing (default values correspond to 250 kb/s)
  public : uint32_t mWhishedBitRate = 250000 ; // In kb/s
  public : uint16_t mBitRatePrescaler = 4 ; // 1...256
  public : uint8_t mPropagationSegment = 5 ; // 1...8
  public : uint8_t mPhaseSegment1 = 5 ; // 1...8
  public : uint8_t mPhaseSegment2 = 5 ;  // 2...8
  public : uint8_t mRJW = 4 ; // 1...4
  public : bool mTripleSampling = false ; // true --> triple sampling, false --> single sampling
  public : bool mBitSettingOk = true ; // The above configuration is correct

//--- Listen only mode
  public : bool mListenOnlyMode = false ; // true --> listen only mode, cannot send any message, false --> normal mode

//--- Self Reception mode
  public : bool mSelfReceptionMode = false ; // true --> sent frame are also received, false --> are not received

//--- Loop Back mode
  public : bool mLoopBackMode = false ; // true --> loop back mode, false --> no loop back

//--- RxFIFO Configuration
  public : typedef enum {k8_0_Filters, k10_6_Filters, k12_12_Filters, k14_18_Filters} tConfiguration ;
  public : tConfiguration mConfiguration = k12_12_Filters ;

//--- Alternate pins
  public : bool mUseAlternateTxPin = false ;
  public : bool mUseAlternateRxPin = false ;

//--- IRQ priority of message interrupt
  public : uint8_t mMessageIRQPriority = 64 ; // 0 --> highest, 255 --> lowest

//--- Receive buffer size
  public : uint16_t mReceiveBufferSize = 32 ;

//--- Transmit buffer size
  public : uint16_t mTransmitBufferSize = 16 ;

//--- Compute actual bit rate
  public : uint32_t actualBitRate (void) const ;

//--- Exact bit rate
  public : bool exactBitRate (void) const ;

//--- Distance between actual bit rate and requested bit rate (in ppm, part-per-million)
  public : uint32_t ppmFromWishedBitRate (void) const ;

//--- Distance of sample point from bit start (in ppc, part-per-cent, denoted by %)
  public : uint32_t samplePointFromBitStart (void) const ;
} ;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
