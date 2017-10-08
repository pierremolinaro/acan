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
  public: explicit ACANSettings (const uint32_t inWhishedBitRate,
                                 const uint32_t inTolerancePPM = 1000) ;

//--- CAN bit timing (default values correspond to 250 kb/s)
  public: uint32_t mWhishedBitRate = 250000 ; // In kb/s
  public: uint16_t mBitRatePrescaler = 4 ; // 1...256
  public: uint8_t mPropagationSegment = 5 ; // 1...8
  public: uint8_t mPhaseSegment1 = 5 ; // 1...8
  public: uint8_t mPhaseSegment2 = 5 ;  // 2...8
  public: uint8_t mRJW = 4 ; // 1...4
  public: bool mTripleSampling = false ; // true --> triple sampling, false --> single sampling
  public: bool mBitConfigurationClosedToWishedRate = true ; // The above configuration is closed to wished bit rate

//--- Listen only mode
  public: bool mListenOnlyMode = false ; // true --> listen only mode, cannot send any message, false --> normal mode

//--- Self Reception mode
  public: bool mSelfReceptionMode = false ; // true --> sent frame are also received, false --> are not received

//--- Loop Back mode
  public: bool mLoopBackMode = false ; // true --> loop back mode, false --> no loop back

//--- RxFIFO Configuration
  public: typedef enum {k8_0_Filters, k10_6_Filters, k12_12_Filters, k14_18_Filters} tConfiguration ;
  public: tConfiguration mConfiguration = k12_12_Filters ;

//--- Alternate pins
  public: bool mUseAlternateTxPin = false ;
  public: bool mUseAlternateRxPin = false ;

//--- IRQ priority of message interrupt
  public: uint8_t mMessageIRQPriority = 64 ; // 0 --> highest, 255 --> lowest

//--- Receive buffer size
  public: uint16_t mReceiveBufferSize = 32 ;

//--- Transmit buffer size
  public: uint16_t mTransmitBufferSize = 16 ;

//--- Compute actual bit rate
  public: uint32_t actualBitRate (void) const ;

//--- Exact bit rate
  public: bool exactBitRate (void) const ;

//--- Distance between actual bit rate and requested bit rate (in ppm, part-per-million)
  public: uint32_t ppmFromWishedBitRate (void) const ;

//--- Distance of sample point from bit start (in ppc, part-per-cent, denoted by %)
  public: uint32_t samplePointFromBitStart (void) const ;

//--- Check CAN bit settings consistency (returns 0 if ok)
  public: uint32_t checkCANBitSettingConsistency (void) const ;

//--- Error codes of checkCANBitSettingConsistency
  public: static const uint32_t kBitRatePrescalerIsZero           = 1 <<  0 ; // mBitRatePrescaler should be >= 1
  public: static const uint32_t kBitRatePrescalerIsGreaterThan256 = 1 <<  1 ; // mBitRatePrescaler should be <= 256
  public: static const uint32_t kPropagationSegmentIsZero         = 1 <<  2 ; // mPropagationSegment should be >= 1
  public: static const uint32_t kPropagationSegmentIsGreaterThan8 = 1 <<  3 ; // mPropagationSegment should be <= 8
  public: static const uint32_t kPhaseSegment1IsZero              = 1 <<  4 ; // mPhaseSegment1 should be >= 1
  public: static const uint32_t kPhaseSegment1IsGreaterThan8      = 1 <<  5 ; // mPhaseSegment1 should be <= 8
  public: static const uint32_t kPhaseSegment2IsZero              = 1 <<  6 ; // mPhaseSegment2 should be >= 1
  public: static const uint32_t kPhaseSegment2IsGreaterThan8      = 1 <<  7 ; // mPhaseSegment7 should be <= 8
  public: static const uint32_t kRJWIsZero                        = 1 <<  8 ; // mRJW should be >= 1
  public: static const uint32_t kRJWIsGreaterThan4                = 1 <<  9 ; // mRJW should be <= 4
  public: static const uint32_t kRJWIsGreaterThanPhaseSegment2    = 1 << 10 ; // mRJW should be <= mPhaseSegment2
} ;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
