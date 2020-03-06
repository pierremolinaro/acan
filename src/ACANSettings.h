//----------------------------------------------------------------------------------------------------------------------
// A simple Arduino Teensy 3.1/3.2/3.5/3.6 CAN driver
// by Pierre Molinaro
// https://github.com/pierremolinaro/acan
//
// This driver is written from FlexCan Library by teachop
// dual CAN support for MK66FX1M0 and updates for MK64FX512 by Pawelsky
// Interrupt driven Rx/Tx with buffers, object oriented callbacks by Collin Kidder
// RTR related code by H4nky84
//
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------

#include <stdint.h>

//----------------------------------------------------------------------------------------------------------------------

class ACANSettings {
//--- Constructor for a given baud rate
  public: explicit ACANSettings (const uint32_t inWhishedBitRate,
                                 const uint32_t inTolerancePPM = 1000) ;

//--- CAN bit timing (default values correspond to 250 kb/s)
  public: uint32_t mWhishedBitRate = 250 * 1000 ; // In kb/s
  public: uint16_t mBitRatePrescaler = 4 ; // 1...256
  public: uint8_t mPropagationSegment = 5 ; // 1...8
  public: uint8_t mPhaseSegment1 = 5 ; // 1...8
  public: uint8_t mPhaseSegment2 = 5 ;  // 2...8
  public: uint8_t mRJW = 4 ; // 1...4
  public: bool mTripleSampling = false ; // true --> triple sampling, false --> single sampling
  public: bool mBitSettingOk = true ; // The above configuration is correct

//--- Listen only mode
  public: bool mListenOnlyMode = false ; // true --> listen only mode, cannot send any message, false --> normal mode

//--- Self Reception mode
  public: bool mSelfReceptionMode = false ; // true --> sent frame are also received, false --> are not received

//--- Loop Back mode
  public: bool mLoopBackMode = false ; // true --> loop back mode, false --> no loop back

//--- RxFIFO Configuration
  public: typedef enum {k8_0_Filters, k10_6_Filters, k12_12_Filters, k14_18_Filters} tConfiguration ;
  public: tConfiguration mConfiguration = k12_12_Filters ;

//--- Tx pin configuration
  public: bool mUseAlternateTxPin = false ;
  public: bool mTxPinIsOpenCollector = false ; // false --> totem pole, true --> open collector

//--- Rx pin configuration
  public: bool mUseAlternateRxPin = false ;
  public: bool mRxPinHasInternalPullUp = false ; // false --> no pullup, true --> pullup enabled


//--- IRQ priority of message interrupt
  public: uint8_t mMessageIRQPriority = 64 ; // 0 --> highest, 255 --> lowest

//--- Receive buffer size
  public: uint16_t mReceiveBufferSize = 32 ;

//--- Transmit buffer size
  public: uint16_t mTransmitBufferSize = 16 ;

//--- Compute actual bit rate
  public: uint32_t actualBitRate (void) const ;

//--- Exact bit rate ?
  public: bool exactBitRate (void) const ;

//--- Distance between actual bit rate and requested bit rate (in ppm, part-per-million)
  public: uint32_t ppmFromWishedBitRate (void) const ;

//--- Distance of sample point from bit start (in ppc, part-per-cent, denoted by %)
  public: uint32_t samplePointFromBitStart (void) const ;

//--- Bit settings are consistent ? (returns 0 if ok)
  public: uint32_t CANBitSettingConsistency (void) const ;

//--- Constants returned by CANBitSettingConsistency
  public: static const uint32_t kBitRatePrescalerIsZero            = 1 <<  0 ;
  public: static const uint32_t kBitRatePrescalerIsGreaterThan256  = 1 <<  1 ;
  public: static const uint32_t kPropagationSegmentIsZero          = 1 <<  2 ;
  public: static const uint32_t kPropagationSegmentIsGreaterThan8  = 1 <<  3 ;
  public: static const uint32_t kPhaseSegment1IsZero               = 1 <<  4 ;
  public: static const uint32_t kPhaseSegment1IsGreaterThan8       = 1 <<  5 ;
  public: static const uint32_t kPhaseSegment2IsZero               = 1 <<  6 ;
  public: static const uint32_t kPhaseSegment2IsGreaterThan8       = 1 <<  7 ;
  public: static const uint32_t kRJWIsZero                         = 1 <<  8 ;
  public: static const uint32_t kRJWIsGreaterThan4                 = 1 <<  9 ;
  public: static const uint32_t kRJWIsGreaterThanPhaseSegment2     = 1 << 10 ;
  public: static const uint32_t kPhaseSegment1Is1AndTripleSampling = 1 << 11 ;
} ;

//----------------------------------------------------------------------------------------------------------------------
