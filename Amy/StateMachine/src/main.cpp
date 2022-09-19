#include "Arduino.h"
#include <ESP.h>
#include "Globals.h"
#include "Config.h"
#include "AFSK.h"
#include "XMDF.h"
#include <SPI.h>
#include <ArduinoJson.h>

// define pins
#define ONOFFHOOK_PIN 14
#define ONOFFHOOK_WHYS_PIN 27
#define DAC1_PIN 25
#define CAL_PIN 33
#define ADC1_PIN 32
#define RING_PIN 35
#define FIRSTRING_PIN 26

// define states
#define ONHOOK_NOCALL 1
#define OFFHOOK_CALLCONN 2
#define OFFHOOK_OUTGOING 3
#define ONHOOK_CALLEND 4
#define AFSK_READY 5
#define AFSK_ALTMARKSPACE 6
#define AFSK_ALLMARKS 7
#define AFSK_DATA 8
#define AFSK_CLEANUP 9
#define AFSK_ERRORx 10


// global variables
static unsigned int phoneState;
static unsigned int AFSKState;
static unsigned int ring;
static unsigned int onoff_hook;
bool spamFlag = false ;                     // used in getCallFromDb() && handleRingDetector() && loop()
const byte relayPin = 15;

//char NameOfPerson[];
//char PhoneNumber[];
static unsigned int TimeOfCall;


// state prototype functions
void phoneState1();
void phoneState2();
void phoneState3();
void phoneState4();
void AFSKState1(DemodState_t lastState, uint32_t mstime);
void AFSKState2(DemodState_t lastState);
void AFSKState3(DemodState_t lastState);
void AFSKState4(DemodState_t lastState);
void AFSKState5(DemodState_t lastState);
void AFSKState6(DemodState_t lastState);
void ICACHE_FLASH_ATTR handleRingDetector(bool force);

char versionNum[] = "SCID ESP32 Decoding v1.0";

Afsk modemTST;

void phoneState1() {

  if (ring == HIGH) {
        // suppress 1st first ring
        // CID state machine
        //    1. channel seizure tones to mark
        //    2. AFSK demodulataor
        //    3. decode cid packet
        //    4. send info to NCID
        static DemodState_t state_last;
        static bool setupRun = false;

        if (!setupRun) {
          // workaround for crash in prolonged setup()
          setup();
          setupRun = true;
        }

        // in debugMode 1 the demod output is raw/unparsed so we exit before the parser stage
        static int charsPrinted = 0;
        if (debugMode == 1)
        {
          static bool runOnce = false;

          if (!runOnce) {
            runOnce = true;
            AFSK_resume();
          }

          if (modemTST.curr_bitReady == true) {
            cli();
            modemTST.curr_bitReady = false;
            sei();
            Serial.print(modemTST.curr_bit);
            charsPrinted++;

            if (charsPrinted == 80) {
              charsPrinted = 0;
              Serial.println();
            }

          }

          return;
        }

        uint32_t ms = millis() ;

        static uint32_t lastUtlityRun = 0 ;
        if (!modemTST.adcSpiExclusive && ((ms - lastUtlityRun) >= 100)) {
          lastUtlityRun = ms ;
        }

        switch (AFSKState) {
          case AFSK_READY:
            AFSKState1(state_last, ms);
            break;
          case AFSK_ALTMARKSPACE:
            AFSKState2(state_last);
            break;
          case AFSK_ALLMARKS:
            AFSKState3(state_last);
            break;
          case AFSK_DATA:
            AFSKState4(state_last);
          case AFSK_CLEANUP:
            AFSKState5(state_last);
            break;
          case AFSK_ERRORx:
            AFSKState6(state_last);
            break;
          default:
            AFSKState = AFSK_READY;
        }
        // if matches with NCID blacklist,
        //    HUP (send a fax tone packet through pin D25)
        // else
        //    let the phone keep ringing
        //    start timer for the ringtime
        //    if ringtime times out
        //        state = ONHOOK_CALLEND
        if ((ONOFFHOOK_PIN == HIGH) || (ONOFFHOOK_WHYS_PIN == HIGH)) {
          phoneState = OFFHOOK_CALLCONN;
          // report to NCID
        }
  }
  else {
    if ((ONOFFHOOK_PIN == HIGH) || (ONOFFHOOK_WHYS_PIN == HIGH)) { // OR if .json file is received from gateway to have NCID make call
      phoneState = OFFHOOK_OUTGOING;
      // report to NCID
    }
  }

}

void AFSKState1(DemodState_t lastState, uint32_t mstime) {

  if (modemTST.demodState == dmREADY) {

    if (lastState != dmREADY) {

      if (debugMode > 1) {
        Serial.println(F("\n\n == == == == == == == == "));
        Serial.print(F("\n > dmREADY"));
      }

    }


    // v0.44
    static uint32_t lastTsScanAtMs = 0 ;
    if (!modemTST.adcSpiExclusive && ((millis() - lastTsScanAtMs) > 100)) {
      AFSK_suspend() ;
      handleRingDetector( true ) ;  // v0.72 writes to TFT so suspend demod required.
      lastTsScanAtMs = mstime;
    }

    static uint32_t lastLdrAtMs = 0 ;
    const uint32_t ldrInterval = 1000UL ;  // x seconds
    if (((mstime - lastLdrAtMs) > ldrInterval) && !modemTST.adcSpiExclusive ) {;
      lastLdrAtMs = mstime ;
    }


    static uint32_t lastStatsDumpAtMs = 0 ;
    const uint32_t statsDumpInterval = 30000UL ; // 30 seconds
    if ((mstime > statsDumpInterval) && ((mstime - lastStatsDumpAtMs) > statsDumpInterval) ) {
      lastStatsDumpAtMs =  mstime ;
    }

    lastState = dmREADY ;
  }

}

void AFSKState2(DemodState_t lastState) {

  if (modemTST.demodState == dmALT_MARK_SPACE) {

    if (lastState != dmALT_MARK_SPACE) {

      if (debugMode > 1) {
        Serial.print(F("\n > dmALT_MARK_SPACE"));
      }

      spamFlag = false;  // also cleared in readMcp3002Ch1()

    }
    lastState = dmALT_MARK_SPACE;
  }

}

void AFSKState3(DemodState_t lastState) {

  if (modemTST.demodState == dmALL_MARKS) {

    if (lastState != dmALL_MARKS) {

      if (debugMode > 1) {
        Serial.print(F("\n > dmALL_MARKS"));
      }

    }

    lastState = dmALL_MARKS;
  }

}

void AFSKState4(DemodState_t lastState) {

  if (modemTST.demodState == dmDATA) {
    //
    // This section handles reading the demodulator bit queue.
    //
    // Avoid blocking code in this section. No Serial Prints except warning/errors else it may
    // not run fast enough to get all the bits from the demodulator and cause a queue overflow.
    //

    if (lastState != dmDATA)  {

      // clean call structure
      for (byte i = 0; i < sizeof(call.allBytes); i++) {
        call.allBytes[i] = 0;
      }

      if (debugMode > 1) {
        Serial.print(F("\n > dmDATA"));
      }

    }

    if (!fifo_isempty_locked(&modemTST.demodFifo)) {
      // something in demodulator queue - take it
      bool inBit = fifo_pop_locked(&modemTST.demodFifo);

      byte rcPbq = parseDemodBitQueue(inBit);

      if (rcPbq == 1) {
        cli();
        modemTST.demodState = dmCLEANUP;
        sei();

        if (debugMode > 1) {
          Serial.println(F( "\n > dmCLEANUP" ));
        }

      }
      else if (rcPbq == 2) {
        cli();
        modemTST.demodError = 3;
        modemTST.demodState = dmERRORx;
        sei();
      }

    }

    lastState =  dmDATA ;
  }

}

void AFSKState5(DemodState_t lastState) {

  if (modemTST.demodState == dmCLEANUP) {
    // we're at the end of the data stream
    // demod can be suspended from here on without checking.

    AFSK_suspend();
    callsThisRun += 1;

    int mql = fifo_maxQuLen(&modemTST.demodFifo) ;
    Serial.print(F("max demod Q len: ")) ;
    Serial.print(mql);

    Serial.print(F("\nCHECKSUM  0x" ));
    Serial.print(call.checksumCalc, HEX);

    if (call.checksumCalc) {
      Serial.print(F("*FAIL*\n"));
    }
    else {
      Serial.print(F("**OK**\n"));
      successCountLoop++;
    }

    Serial.print(F("\ndata: \n"));

    Serial.println(call.date);
    Serial.println(call.number);
    Serial.println (call.name);

    Serial.print(F("\nOK / total: "));
    Serial.print(successCountLoop);
    Serial.print(" / ");
    Serial.print(callsThisRun);
  }

}

void AFSKState6(DemodState_t lastState) {

  if (modemTST.demodState == dmERRORx) {
    AFSK_suspend();

    if (debugMode > 0)
    {

      Serial.println(F(" Parser Error"));
      Serial.println(" ");

      Serial.print(F("Error Flag = " ));
      Serial.println (modemTST.demodError);

      Serial.print(F("Bad Startbits= " ));
      Serial.println(call.CntBadStartBits);

      Serial.print(F("Bad Stopbits= " ));
      Serial.println(call.CntBadStopBits);

      Serial.print(F("Parser Errors= "));
      Serial.println(call.CntParserErrors);

      Serial.print(F("Checksum = "));
      Serial.print(call.checkSumStatus);

    }

    cli();
    modemTST.demodState = dmINIT;
    sei();
    lastState = dmERRORx;
    AFSK_resume();
  }

}

void phoneState2() {
  // begin call 1 recording and timer
  // if DTMF is detected
    // if DTMF is keypad
      //check what numbers are being pressed
    // if DTMF is user or caller hanging up
      phoneState = ONHOOK_CALLEND;
    // if Howler tone is detected
      // report to NCID & let user know
      phoneState = ONHOOK_CALLEND;
}

void phoneState3() {
  // if DTMF solid tone or stutter tone is detected
    // start another timer for user or NCID dialout
      // if keypad DTMF isnt detected within ~s
        phoneState = ONHOOK_CALLEND;
      // if ringtone DTMF is detected
        phoneState = OFFHOOK_CALLCONN;
      // if tone is a stutter
        // send notice to gateway driver about VM
}

void phoneState4() {
  // if phone has been on hook for ~ms
    // stop call ercording and timer
    // report end of call time and other info to ncid
    // second recording to database
    phoneState = ONHOOK_NOCALL;
}

void ICACHE_FLASH_ATTR handleRingDetector(bool force) {
  //
  // force : not used
  //
  //   if in short ring burst and in alternative 1.0 second slot, show "ringing"
  //   else if in new call, show "new call"
  //   else nothing
  //

  const bool localDebug = true;
  // relay handling v0.71

  // globals:
  // inShortRingBurst
  // inLongRingBurst
  // spamFlag

  static bool inShortRingBurstLast = false;
  if (inShortRingBurstLast != inShortRingBurst) {
    if (localDebug) {
      Serial.print("inShortRingBurst= ") ;
      Serial.println(inShortRingBurst) ;
    }
    inShortRingBurstLast = inShortRingBurst;
  }

  static bool inLongRingBurstLast = false;
  if (inLongRingBurstLast != inLongRingBurst) {

    if (localDebug) {
      Serial.print("inLongRingBurst= ") ;
      Serial.println(inLongRingBurst) ;
    }

    if (inLongRingBurst) {
      // transition into longRingBurst
      // clear spamIndicator if set.
      spamFlag = false; // also cleared in loop()
    }
    else {
      // transition out of longRingBurst
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, LOW);
    }
    inLongRingBurstLast = inLongRingBurst;
  }

  if (inLongRingBurst && spamFlag) {
    digitalWrite(relayPin, HIGH);
  }

}

void setup() {

  // open serial connection to output results to serial monitor
  Serial.begin(115200);
  Serial.println(versionNum);

  SPI.begin();

  /* configSetup();
  // count system restarts
  config.runNumber += 1;   
  
  // resave to eeprom
  eepromDump(); 
  eepromFetch();
  PrintEepromVariables(); */

  // set input pins
  pinMode(ONOFFHOOK_PIN, INPUT);
  pinMode(ONOFFHOOK_WHYS_PIN, INPUT);
  pinMode(CAL_PIN, INPUT);
  pinMode(RING_PIN, INPUT);

  // set output pins
  pinMode(FIRSTRING_PIN, OUTPUT);
  pinMode(DAC1_PIN, OUTPUT);

  delay(10000);

  phoneState = ONHOOK_NOCALL;
  AFSKState = AFSK_READY;

  // setup timer and ADC. From here on, SPI slaves (TS, TFT & ADC ) must be interlocked.
  AFSK_init(&modemTST);

  Serial.println(F("Exiting setup()")) ;

  // sending json packages to NCID
  /* StaticJsonDocument<128> doc;

  doc["OnHook"] = 0;
  doc["OffHook"] = 1;
  doc["CallerName"] = NameOfPerson;
  doc["CallerNumber"] = PhoneNumber;
  doc["CallerTime"] = TimeOfCall;

  serializeJson(doc, output); */
}

void loop() {
  
  // phone state machine
  switch (phoneState) {
    case ONHOOK_NOCALL:
      phoneState1();
      break;
    case OFFHOOK_CALLCONN:
      phoneState2();
      break;
    case OFFHOOK_OUTGOING:
      phoneState3();
      break;
    case ONHOOK_CALLEND:
      phoneState4();
      break;
    default:
      phoneState = ONHOOK_NOCALL;
  }

}