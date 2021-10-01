//----------------------------------------------------------------------------------------------------------------------
// A simple Arduino Teensy 3.1/3.2/3.5/3.6 CAN driver
// by Pierre Molinaro
// https://github.com/pierremolinaro/acan
//
//----------------------------------------------------------------------------------------------------------------------
// Teensy 3.1, 3.2 (__MK20DX256__)
//   One CAN module (CAN0)
//        3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
//        32=PTB18=CAN0_TX, 25=PTB19=CAN0_RX (alternative)
//
// Teensy 3.5 (__MK64FX512__)
//   One CAN module (CAN0)
//        3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
//        29=PTB18=CAN0_TX, 30=PTB19=CAN0_RX (alternative)
//
// Teensy 3.6 (__MK66FX1M0__)
//   Two CAN modules (CAN0, CAN1)
//        CAN0: 3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
//        CAN0: 29=PTB18=CAN0_TX, 30=PTB19=CAN0_RX (alternative)
//        CAN1: 33=PTE24=CAN1_TX, 34=PTE25=CAN1_RX (default)
//        CAN1: NOTE: Alternative CAN1 pins are not broken out on Teensy 3.6
//
//----------------------------------------------------------------------------------------------------------------------

#include <ACAN.h>

//----------------------------------------------------------------------------------------------------------------------
//    FlexCAN Register access
//----------------------------------------------------------------------------------------------------------------------

static const uint32_t FLEXCAN0_BASE = 0x40024000 ;
static const uint32_t FLEXCAN1_BASE = 0x400A4000 ;

//----------------------------------------------------------------------------------------------------------------------

typedef volatile uint32_t vuint32_t ;

//----------------------------------------------------------------------------------------------------------------------

#define FLEXCANb_MCR(b)                   (*((vuint32_t *) (b)))
#define FLEXCANb_CTRL1(b)                 (*((vuint32_t *) ((b)+0x04)))
#define FLEXCANb_ECR(b)                   (*((vuint32_t *) ((b)+0x1C)))
#define FLEXCANb_ESR1(b)                  (*((vuint32_t *) ((b)+0x20)))
#define FLEXCANb_IMASK1(b)                (*((vuint32_t *) ((b)+0x28)))
#define FLEXCANb_IFLAG1(b)                (*((vuint32_t *) ((b)+0x30)))
#define FLEXCANb_CTRL2(b)                 (*((vuint32_t *) ((b)+0x34)))
#define FLEXCANb_RXFGMASK(b)              (*((vuint32_t *) ((b)+0x48)))
#define FLEXCANb_RXFIR(b)                 (*((vuint32_t *) ((b)+0x4C)))
#define FLEXCANb_MBn_CS(b, n)             (*((vuint32_t *) ((b)+0x80+(n)*16)))
#define FLEXCANb_MBn_ID(b, n)             (*((vuint32_t *) ((b)+0x84+(n)*16)))
#define FLEXCANb_MBn_WORD0(b, n)          (*((vuint32_t *) ((b)+0x88+(n)*16)))
#define FLEXCANb_MBn_WORD1(b, n)          (*((vuint32_t *) ((b)+0x8C+(n)*16)))
#define FLEXCANb_IDAF(b, n)               (*((vuint32_t *) ((b)+0xE0+(n)*4)))
#define FLEXCANb_MB_MASK(b, n)            (*((vuint32_t *) ((b)+0x880+(n)*4)))

/* Bit definitions and macros for FLEXCAN_MB_CS */
#define FLEXCAN_MB_CS_TIMESTAMP(x)    (((x)&0x0000FFFF)<<0)
#define FLEXCAN_MB_CS_TIMESTAMP_MASK  (0x0000FFFFL)
#define FLEXCAN_MB_CS_LENGTH(x)       (((x)&0x0000000F)<<16)
#define FLEXCAN_MB_CS_RTR             (0x00100000)
#define FLEXCAN_MB_CS_IDE             (0x00200000)
#define FLEXCAN_MB_CS_SRR             (0x00400000)
#define FLEXCAN_MB_CS_CODE(x)         (((x)&0x0000000F)<<24)
#define FLEXCAN_MB_CS_CODE_MASK       (0x0F000000L)
#define FLEXCAN_MB_CS_DLC_MASK        (0x000F0000L)
#define FLEXCAN_MB_CODE_RX_INACTIVE   (0)
#define FLEXCAN_MB_CODE_RX_EMPTY      (4)
#define FLEXCAN_MB_CODE_RX_FULL       (2)
#define FLEXCAN_MB_CODE_RX_OVERRUN    (6)
#define FLEXCAN_MB_CODE_RX_BUSY       (1)

#define FLEXCAN_MB_CS_IDE_BIT_NO      (21)
#define FLEXCAN_MB_CS_RTR_BIT_NO      (20)
#define FLEXCAN_MB_CS_DLC_BIT_NO      (16)

#define FLEXCAN_MB_CODE_TX_BUSY     (0x01)
#define FLEXCAN_MB_CODE_TX_FULL     (0x02)
#define FLEXCAN_MB_CODE_TX_EMPTY    (0x04)
#define FLEXCAN_MB_CODE_TX_OVERRUN  (0x06)
#define FLEXCAN_MB_CODE_TX_INACTIVE  (0x08)
// #define FLEXCAN_MB_CODE_TX_ABORT    (0x09)
#define FLEXCAN_MB_CODE_TX_ONCE      (0x0C)
// #define FLEXCAN_MB_CODE_TX_RESPONSE  (0x0A)
// #define FLEXCAN_MB_CODE_TX_RESPONSE_TEMPO  (0x0E)

#define FLEXCAN_get_code(cs)        (((cs) & FLEXCAN_MB_CS_CODE_MASK)>>24)
#define FLEXCAN_get_length(cs)      (((cs) & FLEXCAN_MB_CS_DLC_MASK)>>16)

/* definitions and macros for FLEXCAN_MCR */
#define FLEXCAN_MCR_MAXMB(x)           (((x)&0x0000007F)<<0)
#define FLEXCAN_MCR_IDAM(x)            (((x)&0x00000003)<<8)
#define FLEXCAN_MCR_MAXMB_MASK         (0x0000007F)
#define FLEXCAN_MCR_IDAM_MASK           (0x00000300)
#define FLEXCAN_MCR_IDAM_BIT_NO        (8)
#define FLEXCAN_MCR_AEN                (0x00001000)
#define FLEXCAN_MCR_LPRIO_EN           (0x00002000)
#define FLEXCAN_MCR_IRMQ               (0x00010000)
#define FLEXCAN_MCR_SRX_DIS            (0x00020000)
#define FLEXCAN_MCR_DOZE               (0x00040000)
#define FLEXCAN_MCR_WAK_SRC            (0x00080000)
#define FLEXCAN_MCR_LPM_ACK            (0x00100000)
#define FLEXCAN_MCR_WRN_EN             (0x00200000)
#define FLEXCAN_MCR_SLF_WAK            (0x00400000)
#define FLEXCAN_MCR_SUPV               (0x00800000)
#define FLEXCAN_MCR_FRZ_ACK            (0x01000000)
#define FLEXCAN_MCR_SOFT_RST           (0x02000000)
#define FLEXCAN_MCR_WAK_MSK            (0x04000000)
#define FLEXCAN_MCR_NOT_RDY            (0x08000000)
#define FLEXCAN_MCR_HALT               (0x10000000)
#define FLEXCAN_MCR_FEN                (0x20000000)
#define FLEXCAN_MCR_FRZ                (0x40000000)
#define FLEXCAN_MCR_MDIS               (0x80000000)

/* Bit definitions and macros for FLEXCAN_CTRL */
#define FLEXCAN_CTRL_PROPSEG(x)        (((uint32_t)(x)) << 0)
#define FLEXCAN_CTRL_LOM               (0x00000008)
#define FLEXCAN_CTRL_LBUF              (0x00000010)
#define FLEXCAN_CTRL_TSYNC             (0x00000020)
#define FLEXCAN_CTRL_BOFF_REC          (0x00000040)
#define FLEXCAN_CTRL_SMP               (0x00000080)
#define FLEXCAN_CTRL_RWRN_MSK          (0x00000400)
#define FLEXCAN_CTRL_TWRN_MSK          (0x00000800)
#define FLEXCAN_CTRL_LPB               (0x00001000)
#define FLEXCAN_CTRL_CLK_SRC           (0x00002000)
#define FLEXCAN_CTRL_ERR_MSK           (0x00004000)
#define FLEXCAN_CTRL_BOFF_MSK          (0x00008000)
#define FLEXCAN_CTRL_PSEG2(x)          (((uint32_t) (x)) << 16)
#define FLEXCAN_CTRL_PSEG1(x)          (((uint32_t) (x)) << 19)
#define FLEXCAN_CTRL_RJW(x)            (((uint32_t) (x)) << 22)
#define FLEXCAN_CTRL_PRESDIV(x)        (((uint32_t) (x)) << 24)

/* Bit definitions and macros for FLEXCAN_MB_ID */
#define FLEXCAN_MB_ID_STD_MASK        (0x1FFC0000L)
#define FLEXCAN_MB_ID_EXT_MASK        (0x1FFFFFFFL)
#define FLEXCAN_MB_ID_IDEXT(x)        (((x)&0x0003FFFF)<<0)
#define FLEXCAN_MB_ID_IDSTD(x)        (((x)&0x000007FF)<<18)
#define FLEXCAN_MB_ID_PRIO(x)         (((x)&0x00000007)<<29)
#define FLEXCAN_MB_ID_PRIO_BIT_NO     (29)
#define FLEXCAN_MB_ID_STD_BIT_NO      (18)
#define FLEXCAN_MB_ID_EXT_BIT_NO      (0)

//----------------------------------------------------------------------------------------------------------------------
//    imin template function
//----------------------------------------------------------------------------------------------------------------------

template <typename T> static inline T imin (const T inA, const T inB) {
  return (inA <= inB) ? inA : inB ;
}

//----------------------------------------------------------------------------------------------------------------------
//    CAN Filter
//----------------------------------------------------------------------------------------------------------------------

static uint32_t defaultMask (const tFrameFormat inFormat) {
  return (inFormat == kExtended) ? 0x1FFFFFFF : 0x7FF ;
}

//----------------------------------------------------------------------------------------------------------------------

static uint32_t computeFilterMask (const tFrameFormat inFormat,
                                   const uint32_t inMask) {
  return
    (1 << 31) | // Test RTR bit
    (1 << 30) | // Test IDE bit
    ((inFormat == kStandard) ? (inMask << 19) : (inMask << 1)) // Test identifier
  ;
}

//----------------------------------------------------------------------------------------------------------------------

static uint32_t computeAcceptanceFilter (const tFrameKind inKind,
                                         const tFrameFormat inFormat,
                                         const uint32_t inMask,
                                         const uint32_t inAcceptance) {
  const uint32_t acceptanceConformanceError = (inFormat == kStandard)
    ? (inAcceptance > 0x7FF)
    : (inAcceptance > 0x1FFFFFFF)
  ;
  const uint32_t maskConformanceError = (inFormat == kStandard)
    ? (inMask > 0x7FF)
    : (inMask > 0x1FFFFFFF)
  ;
//--- inMask & inAcceptance sould be equal to inAcceptance
  const uint32_t maskAndAcceptanceCompabilityError = (inMask & inAcceptance) != inAcceptance ;
//---
  return
    ((inKind == kRemote) ? (1 << 31) : 0) | // Accepts remote or data frames ?
    ((inFormat == kExtended) ? (1 << 30) : 0) | // Accepts standard or extended frames ?
    ((inFormat == kStandard) ? (inAcceptance << 19) : (inAcceptance << 1)) |
    acceptanceConformanceError | // Bit 0 is not used by hardware --> we use it for sting conformance error
    maskConformanceError | maskAndAcceptanceCompabilityError
  ;
}

//----------------------------------------------------------------------------------------------------------------------

ACANPrimaryFilter::ACANPrimaryFilter (const tFrameKind inKind,
                                      const tFrameFormat inFormat,
                                      const ACANCallBackRoutine inCallBackRoutine) :
mFilterMask (computeFilterMask (inFormat, 0)),
mAcceptanceFilter (computeAcceptanceFilter (inKind, inFormat, defaultMask (inFormat), 0)),
mCallBackRoutine (inCallBackRoutine) {
}

//----------------------------------------------------------------------------------------------------------------------

ACANPrimaryFilter::ACANPrimaryFilter (const tFrameKind inKind,
                                      const tFrameFormat inFormat,
                                      const uint32_t inIdentifier,
                                      const ACANCallBackRoutine inCallBackRoutine) :
mFilterMask (computeFilterMask (inFormat, (inFormat == kExtended) ? 0x1FFFFFFF : 0x7FF)),
mAcceptanceFilter (computeAcceptanceFilter (inKind, inFormat, defaultMask (inFormat), inIdentifier)),
mCallBackRoutine (inCallBackRoutine) {
}

//----------------------------------------------------------------------------------------------------------------------

ACANPrimaryFilter::ACANPrimaryFilter (const tFrameKind inKind,
                                      const tFrameFormat inFormat,
                                      const uint32_t inMask,
                                      const uint32_t inAcceptance,
                                      const ACANCallBackRoutine inCallBackRoutine) :
mFilterMask (computeFilterMask (inFormat, inMask)),
mAcceptanceFilter (computeAcceptanceFilter (inKind, inFormat, inMask, inAcceptance)),
mCallBackRoutine (inCallBackRoutine) {
}

//----------------------------------------------------------------------------------------------------------------------

ACANSecondaryFilter::ACANSecondaryFilter (const tFrameKind inKind,
                                          const tFrameFormat inFormat,
                                          const uint32_t inIdentifier,
                                          const ACANCallBackRoutine inCallBackRoutine) :
mSingleAcceptanceFilter (computeAcceptanceFilter (inKind, inFormat, defaultMask (inFormat), inIdentifier)),
mCallBackRoutine (inCallBackRoutine) {
}

//----------------------------------------------------------------------------------------------------------------------
//    FlexCAN Mailboxes configuration
//----------------------------------------------------------------------------------------------------------------------

static const int MB_COUNT = 16 ; // MB count is fixed by hardware

//----------------------------------------------------------------------------------------------------------------------
// FlexCAN is configured for FIFO reception (MCR.FEN bit is set)
// The CTRL2.RFFN field defines the number of Rx FIFO filters (§56.4.12, page 1760)

// RFFN | MB used by RxFIFO | Rx Individual Masks     | Rx Acceptance Filters | Total Filter count
//    0 |    8 (0 ...  7)   |  8 (RXIMR0 ...  RXIMR7) |  0                    |  8
//    1 |   10 (0 ...  9)   | 10 (RXIMR0 ...  RXIMR9) |  6 (10 ... 15)        | 16
//    2 |   12 (0 ... 11)   | 12 (RXIMR0 ... RXIMR11) | 12 (12 ... 23)        | 24
//    3 |   14 (0 ... 13)   | 14 (RXIMR0 ... RXIMR13) | 18 (14 ... 31)        | 32
// Other RFFN values are not available for the Teensy microcontrollers.

//······················································································································

static inline size_t RFFNForConfiguration (const ACANSettings::tConfiguration inConfiguration) {
  return (size_t) inConfiguration ;
}

//······················································································································

static inline size_t primaryFilterCountForConfiguration (const ACANSettings::tConfiguration inConfiguration) {
  return 8 + 2 * (size_t) inConfiguration ;
}

//······················································································································

static inline size_t secondaryFilterCountForConfiguration (const ACANSettings::tConfiguration inConfiguration) {
  return 6 * (size_t) inConfiguration ;
}

//······················································································································

static inline size_t totalFilterCountForConfiguration (const ACANSettings::tConfiguration inConfiguration) {
  return 8 + 8 * (size_t) inConfiguration ;
}

//----------------------------------------------------------------------------------------------------------------------
//    Constructor
//----------------------------------------------------------------------------------------------------------------------

ACAN::ACAN (const uint32_t inFlexcanBaseAddress) :
mFlexcanBaseAddress (inFlexcanBaseAddress) {
}

//----------------------------------------------------------------------------------------------------------------------
//    end
//----------------------------------------------------------------------------------------------------------------------

void ACAN::end (void) {
//--- Disable interrupts
  #if defined(__MK20DX256__)
    NVIC_DISABLE_IRQ (IRQ_CAN_MESSAGE);  // Teensy 3.1 / 3.2
  #elif defined(__MK64FX512__)
    NVIC_DISABLE_IRQ (IRQ_CAN0_MESSAGE);  // Teensy 3.5
  #elif defined(__MK66FX1M0__)
    if(mFlexcanBaseAddress == FLEXCAN0_BASE) {
      NVIC_DISABLE_IRQ (IRQ_CAN0_MESSAGE);  // Teensy 3.6, Can 0
    }else{
      NVIC_DISABLE_IRQ (IRQ_CAN1_MESSAGE); // Teensy 3.6, Can 1
    }
  #endif
//--- Enter freeze mode
  FLEXCANb_MCR (mFlexcanBaseAddress) |= (FLEXCAN_MCR_HALT);
  while (!(FLEXCANb_MCR (mFlexcanBaseAddress) & FLEXCAN_MCR_FRZ_ACK)) ;
//--- Free receive buffer
  delete [] mReceiveBuffer ; mReceiveBuffer = nullptr ;
  mReceiveBufferSize = 0 ;
  mReceiveBufferReadIndex = 0 ;
  mReceiveBufferCount = 0 ;
  mReceiveBufferPeakCount = 0 ;
  mFlexcanRxFIFOFlags = 0 ;
//--- Free transmit buffer
  delete [] mTransmitBuffer ; mTransmitBuffer = nullptr ;
  mTransmitBufferSize = 0 ;
  mTransmitBufferReadIndex = 0 ;
  mTransmitBufferCount = 0 ;
  mTransmitBufferPeakCount = 0 ;
//--- Free callback function array
 delete [] mCallBackFunctionArray ; mCallBackFunctionArray = nullptr ;
 mCallBackFunctionArraySize = 0 ;
}

//----------------------------------------------------------------------------------------------------------------------
//    begin method
//----------------------------------------------------------------------------------------------------------------------

uint32_t ACAN::begin (const ACANSettings & inSettings,
                      const ACANPrimaryFilter inPrimaryFilters [],
                      const uint32_t inPrimaryFilterCount,
                      const ACANSecondaryFilter inSecondaryFilters [],
                      const uint32_t inSecondaryFilterCount) {
  uint32_t errorCode = inSettings.CANBitSettingConsistency () ; // No error code
//--- No configuration if CAN bit settings are incorrect
  if (!inSettings.mBitSettingOk) {
    errorCode |= kCANBitConfiguration ;
  }
  if (0 == errorCode) {
  //---------- Allocate receive buffer
    mReceiveBufferSize = inSettings.mReceiveBufferSize ;
    mReceiveBuffer = new CANMessage [inSettings.mReceiveBufferSize] ;
  //---------- Allocate transmit buffer
    mTransmitBufferSize = inSettings.mTransmitBufferSize ;
    mTransmitBuffer = new CANMessage [inSettings.mTransmitBufferSize] ;
  //---------- Filter count
    const uint32_t MAX_PRIMARY_FILTER_COUNT = primaryFilterCountForConfiguration (inSettings.mConfiguration) ;
    const uint32_t MAX_SECONDARY_FILTER_COUNT = secondaryFilterCountForConfiguration (inSettings.mConfiguration) ;
    const uint32_t primaryFilterCount = imin (inPrimaryFilterCount, MAX_PRIMARY_FILTER_COUNT) ;
    const uint32_t secondaryFilterCount = imin (inSecondaryFilterCount, MAX_SECONDARY_FILTER_COUNT) ;
  //---------- Allocate call back function array
    mCallBackFunctionArraySize = primaryFilterCount + secondaryFilterCount ;
    if (mCallBackFunctionArraySize > 0) {
      mCallBackFunctionArray = new ACANCallBackRoutine [mCallBackFunctionArraySize] ;
      for (uint32_t i=0 ; i<primaryFilterCount ; i++) {
        mCallBackFunctionArray [i] = inPrimaryFilters [i].mCallBackRoutine ;
      }
      for (uint32_t i=0 ; i<secondaryFilterCount ; i++) {
        mCallBackFunctionArray [i + primaryFilterCount] = inSecondaryFilters [i].mCallBackRoutine ;
      }
    }
  //---------- Set up the pins
    const uint32_t TxPinConfiguration =
      PORT_PCR_MUX(2) | // Select function #2
      (inSettings.mTxPinIsOpenCollector ? PORT_PCR_ODE : 0) // Open collector ?
    ;
    const uint32_t RxPinConfiguration =
      PORT_PCR_MUX(2) | // Select function #2
      (inSettings.mRxPinHasInternalPullUp ? (PORT_PCR_PE | PORT_PCR_PS) : 0) // Internal pullup ?
    ;
    #if defined(__MK20DX256__) // Teensy 3.1 / 3.2
    //  3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
    // 32=PTB18=CAN0_TX, 25=PTB19=CAN0_RX (alternative)
      if (inSettings.mUseAlternateTxPin) {
        CORE_PIN32_CONFIG = TxPinConfiguration ;
      }else{
        CORE_PIN3_CONFIG = TxPinConfiguration ;
      }
      if (inSettings.mUseAlternateRxPin) {
        CORE_PIN25_CONFIG = RxPinConfiguration ;
      }else{
        CORE_PIN4_CONFIG = RxPinConfiguration ;
      }
    #elif defined(__MK64FX512__) // Teensy 3.5
    //  3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
    // 29=PTB18=CAN0_TX, 30=PTB19=CAN0_RX (alternative)
      if (inSettings.mUseAlternateTxPin) {
        CORE_PIN29_CONFIG = TxPinConfiguration ;
      }else{
        CORE_PIN3_CONFIG = TxPinConfiguration ;
      }
      if (inSettings.mUseAlternateRxPin) {
        CORE_PIN30_CONFIG = RxPinConfiguration ;
      }else{
        CORE_PIN4_CONFIG = RxPinConfiguration ;
      }
    #elif defined(__MK66FX1M0__) // Teensy 3.6
      if (mFlexcanBaseAddress == FLEXCAN0_BASE) { // aCan0
      //  3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
      // 29=PTB18=CAN0_TX, 30=PTB19=CAN0_RX (alternative)
        if (inSettings.mUseAlternateTxPin) {
          CORE_PIN29_CONFIG = TxPinConfiguration ;
        }else{
          CORE_PIN3_CONFIG = TxPinConfiguration ;
        }
        if (inSettings.mUseAlternateRxPin) {
          CORE_PIN30_CONFIG = RxPinConfiguration ;
        }else{
          CORE_PIN4_CONFIG = RxPinConfiguration ;
        }
      }else{ // Can1
      // 33=PTE24=CAN1_TX, 34=PTE25=CAN1_RX (default)
        CORE_PIN33_CONFIG = TxPinConfiguration ;
        CORE_PIN34_CONFIG = RxPinConfiguration ;
        if (inSettings.mUseAlternateTxPin) {
          errorCode |= kNoAlternateTxPinForCan1 ; // Error
        }
        if (inSettings.mUseAlternateRxPin) {
          errorCode |= kNoAlternateRxPinForCan1 ; // Error
        }
      }
    #endif
  //---------- Power on FlexCAN module, select clock source 16MHz xtal
    OSC0_CR |= OSC_ERCLKEN ; // Enables external reference clock (§28.8.1.1)
    #if defined(__MK20DX256__)
      SIM_SCGC6 |= SIM_SCGC6_FLEXCAN0 ; // Teensy 3.1 / 3.2
    #elif defined(__MK64FX512__)
      SIM_SCGC6 |= SIM_SCGC6_FLEXCAN0 ; // Teensy 3.5
    #elif defined(__MK66FX1M0__)
      if (mFlexcanBaseAddress == FLEXCAN0_BASE) { // Teensy 3.6, Can 0
        SIM_SCGC6 |= SIM_SCGC6_FLEXCAN0 ;
      }else{  // Teensy 3.6, Can 1
        SIM_SCGC3 |= SIM_SCGC3_FLEXCAN1 ;
      }
    #endif
    FLEXCANb_CTRL1 (mFlexcanBaseAddress) &= ~FLEXCAN_CTRL_CLK_SRC; // Use oscillator clock (16 MHz)
  //---------- Enable CAN
    FLEXCANb_MCR (mFlexcanBaseAddress) =
      (1 << 30) | // Enable to enter to freeze mode
      (1 << 23) | // FlexCAN is in supervisor mode
      (15 << 0)   // 16 MB
    ;
    while (FLEXCANb_MCR(mFlexcanBaseAddress) & FLEXCAN_MCR_LPM_ACK) {}
  //---------- Soft reset
    FLEXCANb_MCR(mFlexcanBaseAddress) |= FLEXCAN_MCR_SOFT_RST;
    while (FLEXCANb_MCR(mFlexcanBaseAddress) & FLEXCAN_MCR_SOFT_RST) {}
  //---------- Wait for freeze ack
    while (!(FLEXCANb_MCR(mFlexcanBaseAddress) & FLEXCAN_MCR_FRZ_ACK)) {}
  //---------- Can settings
    FLEXCANb_MCR (mFlexcanBaseAddress) |=
      (inSettings.mSelfReceptionMode ? 0 : FLEXCAN_MCR_SRX_DIS) | // Disable self-reception ?
      FLEXCAN_MCR_FEN  | // Set RxFIFO mode
      FLEXCAN_MCR_IRMQ   // Enable per-mailbox filtering (§56.4.2)
    ;
  //---------- Can bit timing (CTRL1)
    FLEXCANb_CTRL1 (mFlexcanBaseAddress) =
      FLEXCAN_CTRL_PROPSEG (inSettings.mPropagationSegment - 1) |
      FLEXCAN_CTRL_RJW (inSettings.mRJW - 1) |
      FLEXCAN_CTRL_PSEG1 (inSettings.mPhaseSegment1 - 1) |
      FLEXCAN_CTRL_PSEG2 (inSettings.mPhaseSegment2 - 1) |
      FLEXCAN_CTRL_PRESDIV (inSettings.mBitRatePrescaler - 1) |
      (inSettings.mTripleSampling ? FLEXCAN_CTRL_SMP : 0) |
      (inSettings.mLoopBackMode ? FLEXCAN_CTRL_LPB : 0) |
      (inSettings.mListenOnlyMode ? FLEXCAN_CTRL_LOM : 0)
    ;
  //---------- FIFO configuration
    const uint32_t RFFN = RFFNForConfiguration (inSettings.mConfiguration) ;
    const uint32_t TOTAL_FILTER_COUNT = totalFilterCountForConfiguration (inSettings.mConfiguration) ;
  //---------- CTRL2
    FLEXCANb_CTRL2 (mFlexcanBaseAddress) =
      (RFFN << 24) | // Number of RxFIFO
      (0x16 << 19) | // TASD: 0x16 is the default value
      (   0 << 18) | // MRP: Matching starts from RxFIFO and continues on mailboxes
      (   1 << 17) | // RRS: Remote request frame is stored
      (   1 << 16)   // EACEN: RTR bit in mask is always compared
    ;
  //---------- Setup RxFIFO filters
  //--- Default mask
    uint32_t defaultFilterMask = 0 ; // By default, accept any frame
    uint32_t defaultAcceptanceFilter = 0 ;
    if (inPrimaryFilterCount > 0) {
      defaultFilterMask = inPrimaryFilters [0].mFilterMask ;
      defaultAcceptanceFilter = inPrimaryFilters [0].mAcceptanceFilter ;
    }else if (inSecondaryFilterCount > 0) {
      defaultFilterMask = ~1 ;
      defaultAcceptanceFilter = inSecondaryFilters [0].mSingleAcceptanceFilter ;
    }
  //--- Setup primary filters (individual filters in FlexCAN vocabulary)
    if (inPrimaryFilterCount > MAX_PRIMARY_FILTER_COUNT) {
      errorCode |= kTooMuchPrimaryFilters ; // Error, too much primary filters
    }
    mActualPrimaryFilterCount = (uint8_t) primaryFilterCount ;
    mMaxPrimaryFilterCount = (uint8_t) MAX_PRIMARY_FILTER_COUNT ;
    for (uint32_t i=0 ; i<primaryFilterCount ; i++) {
      const uint32_t mask = inPrimaryFilters [i].mFilterMask ;
      const uint32_t acceptance = inPrimaryFilters [i].mAcceptanceFilter ;
      FLEXCANb_MB_MASK (mFlexcanBaseAddress, i) = mask ;
      FLEXCANb_IDAF (mFlexcanBaseAddress, i) = acceptance ;
      if ((acceptance & 1) != 0) {
        errorCode |= kNotConformPrimaryFilter ;
      }
    }
    for (uint32_t i = primaryFilterCount ; i<MAX_PRIMARY_FILTER_COUNT ; i++) {
      FLEXCANb_MB_MASK (mFlexcanBaseAddress, i) = defaultFilterMask ;
      FLEXCANb_IDAF (mFlexcanBaseAddress, i) = defaultAcceptanceFilter ;
    }
  //--- Setup secondary filters (filter mask for Rx individual acceptance filter)
    FLEXCANb_RXFGMASK (mFlexcanBaseAddress) = (inSecondaryFilterCount > 0) ? (~1) : defaultFilterMask ;
    if (inSecondaryFilterCount > MAX_SECONDARY_FILTER_COUNT) {
      errorCode |= kTooMuchSecondaryFilters ;
    }
    for (uint32_t i=0 ; i<secondaryFilterCount ; i++) {
      const uint32_t acceptance = inSecondaryFilters [i].mSingleAcceptanceFilter ;
      FLEXCANb_IDAF (mFlexcanBaseAddress, i + MAX_PRIMARY_FILTER_COUNT) = acceptance ;
      if ((acceptance & 1) != 0) { // Bit 0 is the error flag
        errorCode |= kNotConformSecondaryFilter ;
      }
    }
    for (uint32_t i=MAX_PRIMARY_FILTER_COUNT + secondaryFilterCount ; i<TOTAL_FILTER_COUNT ; i++) {
      FLEXCANb_IDAF (mFlexcanBaseAddress, i) = (inSecondaryFilterCount > 0)
        ? inSecondaryFilters [0].mSingleAcceptanceFilter
        : defaultAcceptanceFilter
      ;
    }
  //---------- Make all other MB inactive
    for (uint32_t i = MAX_PRIMARY_FILTER_COUNT ; i < MB_COUNT ; i++) {
      FLEXCANb_MB_MASK (mFlexcanBaseAddress, i) = 0 ;
      FLEXCANb_MBn_CS (mFlexcanBaseAddress, i) = FLEXCAN_MB_CS_CODE (FLEXCAN_MB_CODE_TX_INACTIVE) ;
    }
  //---------- Start CAN
    FLEXCANb_MCR (mFlexcanBaseAddress) &= ~FLEXCAN_MCR_HALT ;
  //---------- Wait till exit of freeze mode
    while (FLEXCANb_MCR (mFlexcanBaseAddress) & FLEXCAN_MCR_FRZ_ACK) {}
  //----------  Wait till ready
    while (FLEXCANb_MCR (mFlexcanBaseAddress) & FLEXCAN_MCR_NOT_RDY) {}
  //---------- Enable NVIC interrupts
    #if defined(__MK20DX256__)
      NVIC_SET_PRIORITY (IRQ_CAN_MESSAGE, inSettings.mMessageIRQPriority) ; // Teensy 3.1 / 3.2
      NVIC_ENABLE_IRQ (IRQ_CAN_MESSAGE);
    #elif defined(__MK64FX512__)
      NVIC_SET_PRIORITY (IRQ_CAN0_MESSAGE, inSettings.mMessageIRQPriority) ; // Teensy 3.5
      NVIC_ENABLE_IRQ (IRQ_CAN0_MESSAGE);
    #elif defined(__MK66FX1M0__)
      if (mFlexcanBaseAddress == FLEXCAN0_BASE) { // Teensy 3.6, Can 0
        NVIC_SET_PRIORITY (IRQ_CAN0_MESSAGE, inSettings.mMessageIRQPriority) ;
        NVIC_ENABLE_IRQ (IRQ_CAN0_MESSAGE);
      }else{  // Teensy 3.6, Can 1
        NVIC_SET_PRIORITY (IRQ_CAN1_MESSAGE, inSettings.mMessageIRQPriority) ;
        NVIC_ENABLE_IRQ (IRQ_CAN1_MESSAGE) ;
      }
    #endif
  //---------- Enable CAN interrupts (§56.4.10)
    FLEXCANb_IMASK1 (mFlexcanBaseAddress) =
      (1 << 15) | // MB15 (data frame sending)
      (1 << 7) | // RxFIFO Overflow
      (1 << 6) | // RxFIFO Warning: number of messages in FIFO goes from 4 to 5
      (1 << 5)   // Data available in RxFIFO
    ;
  }
//--- Return error code (0 --> no error)
  return errorCode ;
}

//----------------------------------------------------------------------------------------------------------------------
//   RECEPTION
//----------------------------------------------------------------------------------------------------------------------

bool ACAN::receive (CANMessage & outMessage) {
  noInterrupts () ;
    const bool hasMessage = mReceiveBufferCount > 0 ;
    if (hasMessage) {
      outMessage = mReceiveBuffer [mReceiveBufferReadIndex] ;
      mReceiveBufferReadIndex = (mReceiveBufferReadIndex + 1) % mReceiveBufferSize ;
      mReceiveBufferCount -= 1 ;
    }
  interrupts ()
  return hasMessage ;
}

//----------------------------------------------------------------------------------------------------------------------

bool ACAN::dispatchReceivedMessage (const tFilterMatchCallBack inFilterMatchCallBack) {
  CANMessage receivedMessage ;
  const bool hasReceived = receive (receivedMessage) ;
  if (hasReceived) {
    const uint32_t filterIndex = receivedMessage.idx ;
    if (nullptr != inFilterMatchCallBack) {
      inFilterMatchCallBack (filterIndex) ;
    }
    if (filterIndex < mCallBackFunctionArraySize) {
      ACANCallBackRoutine callBackFunction = mCallBackFunctionArray [filterIndex] ;
      if (nullptr != callBackFunction) {
        callBackFunction (receivedMessage) ;
      }
    }
  }
  return hasReceived ;
}

//----------------------------------------------------------------------------------------------------------------------
//   EMISSION
//----------------------------------------------------------------------------------------------------------------------

bool ACAN::tryToSend (const CANMessage & inMessage) {
  const uint32_t firstTxMailBoxIndex = 15 ;
  bool sent = false ;
  if (inMessage.rtr) { // Remote
    for (uint32_t index = mMaxPrimaryFilterCount ; (index < firstTxMailBoxIndex) && !sent ; index++) {
      const uint32_t status = FLEXCAN_get_code (FLEXCANb_MBn_CS (mFlexcanBaseAddress, index)) ;
      switch (status) {
      case FLEXCAN_MB_CODE_TX_INACTIVE : // MB has never sent remote frame
      case FLEXCAN_MB_CODE_TX_EMPTY : // MB has sent a remote frame
      case FLEXCAN_MB_CODE_TX_FULL : // MB has sent a remote frame, and received a frame that did not pass any filter
      case FLEXCAN_MB_CODE_TX_OVERRUN : // MB has sent a remote frame, and received several frames that did not pass any filter
        writeTxRegisters (inMessage, index) ;
        sent = true ;
        break ;
      default:
        break ;
      }
    }
  }else{ // Data
    noInterrupts () ;
    //--- Find an available mailbox
    // Don't compete with the transmit buffer for an inactive mailbox (race condition)
    // Bug fixed in 2.0.1, thanks to wangnick
      if (mTransmitBufferCount == 0) {
        for (uint32_t index = firstTxMailBoxIndex ; (index < MB_COUNT) && !sent ; index++) {
          const uint32_t code = FLEXCAN_get_code (FLEXCANb_MBn_CS (mFlexcanBaseAddress, index)) ;
          if (code == FLEXCAN_MB_CODE_TX_INACTIVE) {
            writeTxRegisters (inMessage, index);
            sent = true ;
          }
        }
      }
    //--- If no mailboxes available, try to buffer it
      if (!sent) {
        sent = mTransmitBufferCount < mTransmitBufferSize ;
        if (sent) {
          uint32_t transmitBufferWriteIndex = mTransmitBufferReadIndex + mTransmitBufferCount ;
          if (transmitBufferWriteIndex >= mTransmitBufferSize) {
            transmitBufferWriteIndex -= mTransmitBufferSize ;
          }
          mTransmitBuffer [transmitBufferWriteIndex] = inMessage ;
          mTransmitBufferCount += 1 ;
        //--- Update max count
          if (mTransmitBufferPeakCount < mTransmitBufferCount) {
            mTransmitBufferPeakCount = mTransmitBufferCount ;
          }
        }else{
          mTransmitBufferPeakCount = mTransmitBufferSize + 1 ;
        }
      }
    interrupts () ;
  }
//---
  return sent ;
}

//----------------------------------------------------------------------------------------------------------------------

void ACAN::writeTxRegisters (const CANMessage & inMessage, const uint32_t inMBIndex) {
//--- Make Tx box inactive
  FLEXCANb_MBn_CS (mFlexcanBaseAddress, inMBIndex) = FLEXCAN_MB_CS_CODE (FLEXCAN_MB_CODE_TX_INACTIVE) ;
//--- Write identifier
  FLEXCANb_MBn_ID (mFlexcanBaseAddress, inMBIndex) = inMessage.ext
    ? (inMessage.id & FLEXCAN_MB_ID_EXT_MASK)
    : FLEXCAN_MB_ID_IDSTD (inMessage.id)
  ;
//--- Write data (registers are big endian, values should be swapped)
  FLEXCANb_MBn_WORD0 (mFlexcanBaseAddress, inMBIndex) = __builtin_bswap32 (inMessage.data32 [0]) ;
  FLEXCANb_MBn_WORD1 (mFlexcanBaseAddress, inMBIndex) = __builtin_bswap32 (inMessage.data32 [1]) ;
//--- Send message
  const uint8_t length = (inMessage.len <= 8) ? inMessage.len : 8 ;
  uint32_t command = FLEXCAN_MB_CS_CODE (FLEXCAN_MB_CODE_TX_ONCE) | FLEXCAN_MB_CS_LENGTH (length) ;
  if (inMessage.rtr) {
    command |= FLEXCAN_MB_CS_RTR ;
  }
  if (inMessage.ext) {
    command |= FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE ;
  }
  FLEXCANb_MBn_CS (mFlexcanBaseAddress, inMBIndex) = command ;
}

//----------------------------------------------------------------------------------------------------------------------
//   MESSAGE INTERRUPT SERVICE ROUTINES
//----------------------------------------------------------------------------------------------------------------------

void ACAN::readRxRegisters (CANMessage & outMessage) {
//--- Get identifier, ext, rtr and len
  const uint32_t dlc = FLEXCANb_MBn_CS (mFlexcanBaseAddress, 0) ;
  outMessage.len = FLEXCAN_get_length (dlc) ;
  if (outMessage.len > 8) {
    outMessage.len = 8 ;
  }
  outMessage.ext = (dlc & FLEXCAN_MB_CS_IDE) != 0 ;
  outMessage.rtr = (dlc & FLEXCAN_MB_CS_RTR) != 0 ;
  outMessage.id  = FLEXCANb_MBn_ID (mFlexcanBaseAddress, 0) & FLEXCAN_MB_ID_EXT_MASK ;
  if (!outMessage.ext) {
    outMessage.id >>= FLEXCAN_MB_ID_STD_BIT_NO ;
  }
//-- Get data (registers are big endian, values should be swapped)
  outMessage.data32 [0] = __builtin_bswap32 (FLEXCANb_MBn_WORD0 (mFlexcanBaseAddress, 0)) ;
  outMessage.data32 [1] = __builtin_bswap32 (FLEXCANb_MBn_WORD1 (mFlexcanBaseAddress, 0)) ;
//--- Zero unused data entries
  for (uint32_t i = outMessage.len ; i < 8 ; i++) {
    outMessage.data [i] = 0 ;
  }
//--- Get filter index
  outMessage.idx = (uint8_t) FLEXCANb_RXFIR (mFlexcanBaseAddress) ;
  if (outMessage.idx >= mMaxPrimaryFilterCount) {
    outMessage.idx -= mMaxPrimaryFilterCount - mActualPrimaryFilterCount ;
  }
}

//----------------------------------------------------------------------------------------------------------------------

void ACAN::message_isr (void) {
  const uint32_t status = FLEXCANb_IFLAG1 (mFlexcanBaseAddress) ;
//--- A trame has been received in RxFIFO ?
  if ((status & (1 << 5)) != 0) {
    CANMessage message ;
    readRxRegisters (message) ;
    if (mReceiveBufferCount == mReceiveBufferSize) { // Overflow! Receive buffer is full
      mReceiveBufferPeakCount = mReceiveBufferSize + 1 ; // Mark overflow
    }else{
      uint32_t receiveBufferWriteIndex = mReceiveBufferReadIndex + mReceiveBufferCount ;
      if (receiveBufferWriteIndex >= mReceiveBufferSize) {
        receiveBufferWriteIndex -= mReceiveBufferSize ;
      }
      mReceiveBuffer [receiveBufferWriteIndex] = message ;
      mReceiveBufferCount += 1 ;
      if (mReceiveBufferCount > mReceiveBufferPeakCount) {
        mReceiveBufferPeakCount = mReceiveBufferCount ;
      }
    }
  }
//--- RxFIFO warning ? It occurs when the number of messages goes from 4 to 5
  if ((status & (1 << 6)) != 0) {
    mFlexcanRxFIFOFlags |= 1 ;
  }
//--- RxFIFO Overflow ?
  if ((status & (1 << 7)) != 0) {
    mFlexcanRxFIFOFlags |= 2 ;
  }
//--- Handle Tx MBs
  if (mTransmitBufferCount > 0) { // There is a frame in the queue to send
    const uint32_t firstTxMailBoxIndex = 15 ;
    uint32_t s = (status >> firstTxMailBoxIndex) & 0x3 ;
    uint32_t mb = firstTxMailBoxIndex ;
    while (s != 0) {
      if ((s & 1) != 0) { // Has this mailbox triggered an interrupt?
        const uint32_t code = FLEXCAN_get_code (FLEXCANb_MBn_CS (mFlexcanBaseAddress, mb));
        if (code == FLEXCAN_MB_CODE_TX_INACTIVE) {
          writeTxRegisters (mTransmitBuffer [mTransmitBufferReadIndex], mb);
          mTransmitBufferReadIndex = (mTransmitBufferReadIndex + 1) % mTransmitBufferSize ;
          mTransmitBufferCount -= 1 ;
        }
      }
      s >>= 1 ;
      mb += 1 ;
    }
  }
//--- Writing its value back to itself clears all flags
  FLEXCANb_IFLAG1 (mFlexcanBaseAddress) = status ;
}

//----------------------------------------------------------------------------------------------------------------------

void can0_message_isr (void) {
  ACAN::can0.message_isr () ;
}

//----------------------------------------------------------------------------------------------------------------------

#ifdef __MK66FX1M0__
  void can1_message_isr (void) {
    ACAN::can1.message_isr () ;
  }
#endif

//----------------------------------------------------------------------------------------------------------------------
//   Controller state
//----------------------------------------------------------------------------------------------------------------------

tControllerState ACAN::controllerState (void) const {
  uint32_t state = (FLEXCANb_ESR1 (mFlexcanBaseAddress) >> 4) & 0x03 ;
//--- Bus-off state is value 2 or value 3
  if (state == 3) {
    state = 2 ;
  }
  return (tControllerState) state ;
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ACAN::receiveErrorCounter (void) const {
  return FLEXCANb_ECR (mFlexcanBaseAddress) >> 8 ;
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ACAN::transmitErrorCounter (void) const {
//--- In bus-off state, TXERRCNT field of CANx_ECR register does not reflect transmit error count: we force 256
  const tControllerState state = controllerState () ;
  return (state == kBusOff) ? 256 : (FLEXCANb_ECR (mFlexcanBaseAddress) & 0xFF) ;
}

//----------------------------------------------------------------------------------------------------------------------
//   Drivers as global variables
//----------------------------------------------------------------------------------------------------------------------

ACAN ACAN::can0 (FLEXCAN0_BASE) ;

//······················································································································

#ifdef __MK66FX1M0__
  ACAN ACAN::can1 (FLEXCAN1_BASE) ;
#endif

//----------------------------------------------------------------------------------------------------------------------
