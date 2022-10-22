#ifndef AFSK_H
#define AFSK_H

#include "FIFO.h"
#include "Arduino.h"

#define SIN_LEN 512

// misc constants
#define FREQUENCY_CORRECTION 0
// #define ADC_PORT PORTC
// #define ADC_DDR  DDRC
#define CPU_FREQ F_CPU

#define DUAL_XOR(bits1, bits2) ((((bits1)^(bits2)) & 0x03) == 0x03)
#define SIGNAL_TRANSITIONED(bits) DUAL_XOR((bits), (bits) >> 2)

#define SAMPLERATE 9600
#define BITRATE    1200
#define SAMPLESPERBIT (SAMPLERATE / BITRATE)

// mark /& space frequencies vary between standards.
//#define MARK_FREQ  1200
//#define SPACE_FREQ 2200
#define MARK_FREQ  1300   // V23 from http://www.cmlmicro.com/assets/MX602DB_R4.pdf
#define SPACE_FREQ 2100

#define PHASE_BITS   8                              // How much to increment phase counter each sample
#define PHASE_INC    1                              // Nudge by an eigth of a sample each adjustment
#define PHASE_MAX    (SAMPLESPERBIT * PHASE_BITS)   // Resolution of our phase counter = 64
#define PHASE_THRESHOLD  (PHASE_MAX / 2)            // Target transition point of our phase window

// esp32 TX2 pin
const byte  ADC_CS_PIN = 17;     // ADC Chip Select Pin

const bool SPACEA = false;  // SPACE - possible conflict with IR library
const bool MARKA = true;    // MARK - possible conflict with IR library

extern volatile uint32_t demodLastActiveAtMs;
// extern volatile uint16_t adcValueCh1;
extern volatile bool inShortRingBurst;
extern volatile bool inLongRingBurst;

typedef enum  {
  dmINIT = 1,
  dmREADY,
  dmALT_MARK_SPACE,
  dmALL_MARKS,
  dmDATA,
  dmCLEANUP,
  dmERRORx
} DemodState_t;

typedef struct Afsk
{

  // Demodulation values
  FIFOBuffer delayFifo;                   // Delayed FIFO for frequency discrimination
  int8_t delayBuf[SAMPLESPERBIT / 2 + 1]; // Actual data storage for said FIFO

  FIFOBuffer demodFifo;
  uint8_t demodBuf[20];                  // X byte queue between demodulator and parser.  // te_54 was 40

  int16_t iirX[2];                        // IIR Filter X cells
  int16_t iirY[2];                        // IIR Filter Y cells

  uint8_t sampledBits;                    // Bits sampled by the demodulator (at ADC speed)
  int8_t currentPhase;                    // Current phase of the demodulator
  uint8_t actualBits;                     // Actual found bits at correct bitrate

  volatile bool curr_bit;                // latest bit found in input stream
  volatile bool last_bit;                // old curr_bit
  volatile bool curr_bitReady;           // is current Bit Ready? set by AFSK.cpp. Unset by consumer.
  volatile bool adcSpiExclusive;         // true: other devices cant grab the SPI bus, false otherwise
  volatile byte counter1111;              //  how many uninterrupted  111111's received since last 0 - capped at 255
  volatile unsigned int counter0000;      //  how many uninterrupted  000000's received since last 1 - capped at 0xFFFF
  volatile byte counter0101;             //  how many uninterrupted  010101's received - capped at 255
  volatile unsigned int samplesIn_dmALT_MARK_SPACE;
  volatile DemodState_t demodState;      // state of demodulator
  volatile byte demodError;              // 3 = badPacketHeader; 4 = stuckInAltMarkSpaces; 5= demodQueueFull; 6 = endOfDataNotHandled

  volatile bool demodBusy;                // true to lock other activities out during critical state.
  volatile uint8_t byteWork;              // for byte Assembler part
  volatile uint8_t byteWorkBitIndex;      // index into current byte  0=start; 1-8 bits; 9 = stop. Only 1-8 are stored.
  volatile uint16_t bitStuffDiscarded;
  volatile uint16_t CntBadStartBits;
} Afsk;

#define DIV_ROUND(dividend, divisor)  (((dividend) + (divisor) / 2) / (divisor))
#define MARK_INC   (uint16_t)(DIV_ROUND(SIN_LEN * (uint32_t)MARK_FREQ, CONFIG_AFSK_DAC_SAMPLERATE))
#define SPACE_INC  (uint16_t)(DIV_ROUND(SIN_LEN * (uint32_t)SPACE_FREQ, CONFIG_AFSK_DAC_SAMPLERATE))

// prototypes

void AFSK_adc_isr(Afsk *afsk, int8_t currentSample);
void AFSK_init(Afsk *afsk);
void AFSK_suspend();
void AFSK_resume();
//void ringDetector();


#endif