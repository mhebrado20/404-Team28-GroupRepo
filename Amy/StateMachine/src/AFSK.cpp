#include "Globals.h"
#include "AFSK.h"
#include <SPI.h>

Afsk *AFSK_modem;

volatile uint32_t demodLastActiveAtMs = 0 ;
volatile uint16_t adcValueCh1 = 0 ;

volatile bool inShortRingBurst = false ;
volatile bool inLongRingBurst = false ;

bool demodRun = true ; // v0.65 used to suspend demodulator code. Timer still runs.
volatile bool demodActive = false ;  // interlock to ensure demod run complete

void onTimer();
void startTimer();
void endTimer();
void getADCSample();

// a pointer variable timer of the type hw_timer_t in order to configure the timer
hw_timer_t *timer = NULL;

void onTimer(){
  static unsigned int timeCounter = 1;

  Serial.print("onTimer ");
  Serial.print(timeCounter);
  Serial.print(" at ");
  Serial.print(millis());
  Serial.println(" ms");

  // need to change if statement
  if (timeCounter == 10) {
    endTimer();
  }

  timeCounter++;
}

void startTimer() {
  Serial.println("Start timer");
  // timer 0, CP = 1 ns, count up
  timer = timerBegin(0, 80, true);
  // edge triggered
  timerAttachInterrupt(timer, &onTimer, true);
  // 1 s
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void endTimer() {
  timerEnd(timer);
  timer = NULL;
  Serial.println("Timer stopped.");
}

void ICACHE_RAM_ATTR getAdcSample()
{
  // called by timer1 (9600Hz) set up in AFSK_resume() and supended in AFSK_suspend()
  // MCP2003 code loosely based on
  // https://rheingoldheavy.com/mcp3008-tutorial-02-sampling-dc-voltage/
  // but modified for the mcp3002 and to return an 8 significant bit number shifted to range -128 to 127.

  // routine supended when SPI devices (TS and TFT) which are synchronous with the main loop() are active,
  // tp prevent contention.

  // v 0.65 - we let the timer run but don't act on it. Minimise timer suspend/resume overhead.
  if (!demodRun) {
    return;
  }

  demodActive = true;  // interlock

  const byte MCP3002Ch0 = 0b01101000;  // start bit + openended conversion + chan 0 + MSBF (see data sheet)
  const byte MCP3002Ch1 = 0b01111000;  // start bit + openended conversion + chan 1 + MSBF (see data sheet)
  byte dataMsb0;
  byte dataLsb0;
  byte dataMsb1;
  byte dataLsb1;

  int sample8BitMidZero;
  static uint32_t runCount = 0;

  if (runCount >= 100000UL) {
    // approx 10 seconds
    demodLastActiveAtMs = millis();
    runCount = 0;
  }
  else {
    runCount++;
  }

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  digitalWrite(ADC_CS_PIN, LOW);
  dataMsb0 = SPI.transfer(MCP3002Ch0) & 0x03;  // 2 most significant digits in this byte
  dataLsb0 = SPI.transfer(0);                  // Push remaining data and get LSB byte return
  digitalWrite(ADC_CS_PIN, HIGH);

  digitalWrite(ADC_CS_PIN, LOW);
  dataMsb1 = SPI.transfer(MCP3002Ch1) & 0x03;  // 2 most significant digits in this byte
  dataLsb1 = SPI.transfer(0);                  // Push remaining data and get LSB byte return
  digitalWrite(ADC_CS_PIN, HIGH);

  SPI.endTransaction();

  sample8BitMidZero = (int16_t)((dataMsb0 << 6) | (dataLsb0 >> 2)) - 128;  // build 8 significant bit number in range -128 to 127
  adcValueCh1 = (int16_t)( dataMsb1 << 8 ) | dataLsb1;   // 0..1023  ring detector

  AFSK_adc_isr(AFSK_modem, sample8BitMidZero);

  // v0.71 call ringDetector() every 10 times ( ~1ms @9600Hz)
  static byte ringDetectorItCount = 0;
  if (ringDetectorItCount < 10) {
    ringDetectorItCount++;
  }
  else {
    ringDetectorItCount = 0;
    ringDetector();
  }

  demodActive = false;
}




// ==============
// AFSK_init()
// ==============


void ICACHE_FLASH_ATTR AFSK_init(Afsk *afsk) {
  // Clean modem struct memory
  memset(afsk, 0, sizeof(*afsk));
  AFSK_modem = afsk;

  // Initialise FIFO buffers
  fifo_init(&afsk->delayFifo, (uint8_t *)afsk->delayBuf, sizeof(afsk->delayBuf));

  fifo_init(&afsk->demodFifo, (uint8_t *)afsk->demodBuf, sizeof(afsk->demodBuf));

  // Fill delay FIFO with zeroes
  for (int i = 0; i < (SAMPLESPERBIT / 2); i++) {
    fifo_push(&afsk->delayFifo, 0);
  }

  afsk->demodState = dmINIT;
  // demodInitialised = true ;

  pinMode(ADC_CS_PIN, OUTPUT);

  // AFSK_resume() ;  // timer1 initial set to start demod.
  // v0.65
  
  startTimer();
}

// ==============
// AFSK_resume()
// ==============

void ICACHE_RAM_ATTR AFSK_resume() {

  if (debugMode == 3) {
    swSerial.println(F("in AFSK_resume()"));
  }
  // timer1: http://onetechpulse.com/esp8266-microsecond-delay-timer/
  // v0.65  only enable/disable now required ?

  demodRun = true ;
}

// ==============
// AFSK_suspend ()
// ==============

void ICACHE_RAM_ATTR AFSK_suspend() {

  if (debugMode == 3) {
    swSerial.println(F("in AFSK_suspend()"));
  }

  demodRun = false ;

  while (demodActive) {} // wait

}



// ==============
// AFSK_adc_isr()
// ==============


void ICACHE_RAM_ATTR AFSK_adc_isr(Afsk *afsk, int8_t currentSample) {
  // To determine the received frequency, and thereby
  // the bit of the sample, we multiply the sample by
  // a sample delayed by (samples per bit / 2).
  // We then lowpass-filter the samples with a
  // Chebyshev filter. The lowpass filtering serves
  // to "smooth out" the variations in the samples.

  afsk->iirX[0] = afsk->iirX[1];
  afsk->iirX[1] = ((int8_t)fifo_pop(&afsk->delayFifo) * currentSample) >> 2;

  afsk->iirY[0] = afsk->iirY[1];

  afsk->iirY[1] = afsk->iirX[0] + afsk->iirX[1] + (afsk->iirY[0] >> 1); // Chebyshev filter


  // We put the sampled bit in a delay-line:
  // First we bitshift everything 1 left
  afsk->sampledBits <<= 1;
  // And then add the sampled bit to our delay line
  afsk->sampledBits |= (afsk->iirY[1] > 0) ? 1 : 0;

  // Put the current raw sample in the delay FIFO
  fifo_push(&afsk->delayFifo, currentSample);

  // We need to check whether there is a signal transition.
  // If there is, we can recalibrate the phase of our
  // sampler to stay in sync with the transmitter. A bit of
  // explanation is required to understand how this works.
  // Since we have PHASE_MAX/PHASE_BITS = 8 samples per bit,
  // we employ a phase counter (currentPhase), that increments
  // by PHASE_BITS everytime a sample is captured. When this
  // counter reaches PHASE_MAX, it wraps around by modulus
  // PHASE_MAX. We then look at the last three samples we
  // captured and determine if the bit was a one or a zero.
  //
  // This gives us a "window" looking into the stream of
  // samples coming from the ADC. Sort of like this:
  //
  //   Past                                      Future
  //       0000000011111111000000001111111100000000
  //                   |________|
  //                       ||
  //                     Window
  //
  // Every time we detect a signal transition, we adjust
  // where this window is positioned little. How much we
  // adjust it is defined by PHASE_INC. If our current phase
  // phase counter value is less than half of PHASE_MAX (ie,
  // the window size) when a signal transition is detected,
  // add PHASE_INC to our phase counter, effectively moving
  // the window a little bit backward (to the left in the
  // illustration), inversely, if the phase counter is greater
  // than half of PHASE_MAX, we move it forward a little.
  // This way, our "window" is constantly seeking to position
  // it's center at the bit transitions. Thus, we synchronise
  // our timing to the transmitter, even if it's timing is
  // a little off compared to our own.

  // te_47 no change to currentPhase if at middle point.
  if (SIGNAL_TRANSITIONED(afsk->sampledBits)) {

    if (afsk->currentPhase < PHASE_THRESHOLD) {
      afsk->currentPhase += PHASE_INC;
    } else if (afsk->currentPhase > PHASE_THRESHOLD) {
      afsk->currentPhase -= PHASE_INC;
    }

  }

  // We increment our phase counter
  afsk->currentPhase += PHASE_BITS;

  // Check if we have reached the end of
  // our sampling window.
  if (afsk->currentPhase >= PHASE_MAX) {
    // If we have, wrap around our phase
    // counter by modulus
    afsk->currentPhase = ( afsk->currentPhase + PHASE_MAX ) % PHASE_MAX;     // could be slow ? ensure not negative

    // Bitshift to make room for the next
    // bit in our stream of demodulated bits
    afsk->actualBits <<= 1;


    // We determine the actual bit value by reading
    // the last 3 sampled bits. If there is three or
    // more 1's, we will assume that the transmitter
    // sent us a one, otherwise we assume a zero
    uint8_t bits = afsk->sampledBits & 0x07;
    if (bits == 0x07 || // 111
        bits == 0x06 || // 110
        bits == 0x05 || // 101
        bits == 0x03    // 011
       ) {
      afsk->actualBits |= 1;
    }

    afsk->curr_bit = !( afsk->actualBits & 1 ) ;      // inverted ?
    afsk->curr_bitReady = true ;    // possible test to check it is false, otherwise increment unconsumed bit count.




    if ((afsk->curr_bit == afsk->last_bit)  && (afsk->curr_bit == MARKA)) {

      if (afsk->counter1111 < 255) {
        afsk->counter1111++;
      }

    }
    else {
      afsk->counter1111 = 0 ;
    }

    if ((afsk->curr_bit == afsk->last_bit)  && (afsk->curr_bit == SPACEA)) {

      if (afsk->counter0000 < 0xFFFF) {
        afsk->counter0000++;
      }

    }
    else {
      afsk->counter0000 = 0 ;
    }

    if (afsk->curr_bit != afsk->last_bit) {

      if (afsk->counter0101 < 255) {
        afsk->counter0101++;
      }
      
    }
    else afsk->counter0101 = 0 ;


    if (afsk->demodState == dmINIT) {
      afsk->counter1111 = 0;
      afsk->counter0000 = 0;
      afsk->counter0101 = 0;
      fifo_flush(&afsk->demodFifo);
      afsk->demodError = 0;
      afsk->demodState = dmREADY;
    }

    // in debugMode 1  the data is presented as raw without parsing, so quit here (after setting demodState = dmREADY)  v0.64

    if (debugMode == 1) {
      return;
    }


    if ( afsk->demodState == dmREADY ) {
      if ( afsk->counter0101 >= 20 ) {   // allow stabilisation
        // afsk->entered_dmALT_MARK_SPACE_atTicks = adcIsr_ticks ;
        afsk->samplesIn_dmALT_MARK_SPACE = 0 ;
        afsk->demodState = dmALT_MARK_SPACE ;
      }
    }

    if ( afsk->demodState == dmALT_MARK_SPACE ) {
      if ( afsk->counter1111 > 10  ) {
        afsk->demodState = dmALL_MARKS ;
      }
      else if ( afsk->samplesIn_dmALT_MARK_SPACE > 400 ) {
        // we're stuck here so terminate
        afsk->demodError = 4 ;
        afsk->demodState = dmERRORx ;
      }
      else {
        afsk->samplesIn_dmALT_MARK_SPACE ++ ;
      }
    }

    if ( afsk->demodState == dmALL_MARKS ) {
      if ( afsk->counter1111 == 0 ) {
        afsk->demodState = dmDATA ;
      }
    }

    if ( afsk->demodState == dmDATA ) {
      if ( ! fifo_isfull_locked( &afsk->demodFifo ) ) {   // locked version not required in ISR ?
        fifo_push_locked( &afsk->demodFifo,  afsk->curr_bit  ) ;
      }
      else {
        afsk->demodError = 5 ;
        afsk->demodState = dmERRORx ;    // demod queue full
      }
      if ( afsk->counter0000 >= 200 ) {   // if we've shot wildly over the end  // te_55 was 4000
        afsk->demodError = 6 ;
        afsk->demodState = dmERRORx ;    // state after DATA normally set in header parser (should set error code ? )
      }
    }

    afsk->last_bit =  afsk->curr_bit ;

    afsk->adcSpiExclusive =  afsk->demodState == dmALT_MARK_SPACE || afsk->demodState == dmALL_MARKS
                             || afsk->demodState == dmDATA  || ( afsk->demodState == dmREADY && afsk->counter0101 > 2 ) ;

  }
}


// ==============
// ringDetector()
// ==============

void ICACHE_RAM_ATTR ringDetector() {
  //
  // Called once every 10 iterations of 9600Hz  ( approx 1 time per mS )
  // average analogRead value over over 4 samples used to identify ring pulse (25Hz one wave - 40mS)
  // if 4 or more ring pulses counted in a 150mS time slot, we are in a short ring burst.
  //
  // May need to be tuned to a specific installation for reliable detection of ring bursts.

  const bool localDebug = false ;

  static uint32_t pulseStart ;
  static uint32_t pulseEnd ;
  bool inPulse ;
  static bool inPulseLast ;
  static uint32_t av = 0 ;
  const byte sampleIntevalMs = 1 ;
  const byte numberOfSamples = 4 ;
  static uint32_t lastSampleAtMs = 0 ;
  static uint32_t shortRingConfirmedAtMs = 0;
  static uint32_t pulseCountInPeriod = 0 ;
  static uint32_t sampleCounter = 0 ; // reset at end of pulseCountCollectionPeriod
  const uint16_t pulseCountCollectionPeriod = 200 ;  // 150 mS @ sample rate 1 per ms.

  uint32_t ms = millis() ;


  sampleCounter ++ ;

  av =  ( ( numberOfSamples - 1 ) * av ) / numberOfSamples     +  adcValueCh1 / numberOfSamples ;   // average calculation

  if ( localDebug && false ) {
    /*
      swSerial.print( F("ar= " )) ;
      swSerial.print( ar ) ;
      swSerial.print( F("; av= " )) ;
      swSerial.println( av ) ;
    */
  }

  ( av > 200 ) ? inPulse = true : inPulse = false ;

  if ( inPulseLast == false && inPulse == true ) {
    if ( localDebug ) {
      /*
        swSerial.print( F("Space= " )) ;
        swSerial.println( ms - pulseEnd ) ;
      */
    }
    pulseStart = ms ;
    pulseCountInPeriod ++ ;
  }

  if ( inPulse == false && inPulseLast == true ) {
    if ( localDebug ) {
      /*
        swSerial.print( F("Mark = " )) ;
        swSerial.println( ms - pulseStart ) ;
      */
    }
    pulseEnd = ms ;
  }

  inPulseLast = inPulse ;

  if ( sampleCounter >= pulseCountCollectionPeriod ) {
    static uint32_t collectionPeriodStartLastAtMs = 0 ;

    if ( pulseCountInPeriod >= 4 && pulseCountInPeriod < 40 ) {   // upper bound not tested - attempt to exclude outgoing dtmf
      shortRingConfirmedAtMs = ms ;
      if ( localDebug ) {
        /*
          swSerial.print( F("pulses in pulseCountCollectionPeriod =" )) ;
          swSerial.print( pulseCountInPeriod ) ;
          swSerial.print( F(" ; collectionPeriod ms= " )) ;
          swSerial.println( ms - collectionPeriodStartLastAtMs ) ;
        */
      }
    }
    collectionPeriodStartLastAtMs = ms ;
    sampleCounter = 0 ;
    pulseCountInPeriod = 0 ;
  }


  // globals
  inShortRingBurst = ( ms > 5000 &&  ms - shortRingConfirmedAtMs < 1700 ) ;  // prevent trigger on system start.
  inLongRingBurst = ( ms > 5000 &&  ms - shortRingConfirmedAtMs < 5000 ) ;  // prevent trigger on system start.

}