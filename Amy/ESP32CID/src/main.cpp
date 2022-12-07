#include "Arduino.h"
#include <ESP.h>
#include "Globals.h"
#include "Config.h"
#include "AFSK.h"
#include "XMDF.h"
#include <ArduinoJson.h>
#include <PhoneDTMF.h>
#include <driver/adc.h>
#include "Adafruit_FONA.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <stdio.h>
#include <I2SMEMSSampler.h>
#include <ADCSampler.h>
#include <I2SOutput.h>
#include <DACOutput.h>
#include <WAVFileReader.h>
#include <WAVFileWriter.h>
#include <Streaming.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// define pins
#define ONOFFHOOK_PIN 14
#define ONOFFHOOK_WHYS_PIN 27
#define DAC1_PIN 25
#define CAL_PIN 33
#define ADC1_PIN 32
#define RING_PIN 35
#define FIRSTRING_PIN 26
#define WSLED_PIN 15
#define LEDNUM 3
// analog input pin might be implemented later
// #define ANALOGINPUT_PIN 34

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

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 32 // OLED height, in pixels


// global variables
byte debugMode = 2 ;  // updated in setup()   
unsigned long callsThisRun = 0 ;
uint32_t successCountLoop = 0 ;
static unsigned int phoneState;
static unsigned int AFSKState;
static unsigned int ring;
static unsigned int onoff_hook;
bool numberMatchesBL = false;
bool callOngoing;

// state prototype functions
void cidSM();
void onHookNoCall();
void offHookCallConn();
void offHookOutgoing();
void onHookCallEnd();
void cidStateMachine();
void detectDTMF();
void AFSKReady(DemodState_t lastAFSKState, uint32_t mstime);
void AFSKAltMark(DemodState_t lastAFSKState);
void AFSKAllMarks(DemodState_t lastAFSKState);
void AFSKData(DemodState_t lastAFSKState);
void AFSKCleanUp(DemodState_t lastAFSKState);
void AFSKErrorx(DemodState_t lastAFSKState);
//void ICACHE_FLASH_ATTR handleRingDetector(bool force);

char versionNum[] = "SCID ESP32 Decoding v1.0";

Afsk modemTST;
PhoneDTMF dtmf;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LEDNUM, WSLED_PIN, NEO_GRB + NEO_KHZ800);
I2SSampler *input = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_4, i2s_adc_config);

/************* Timers *************/
hw_timer_t *Timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
unsigned int timeCounter = 1;

void IRAM_ATTR onTimer() { 
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

/************* CID State Machine *************/
/* 1. channel seizure tones to mark
   2. AFSK demodulataor
   3. decode cid packet
   4. send info to NCID
*/
void cidSM() {
  static DemodState_t state_last;
  static bool setupRun = false;

  if (!setupRun) {
    // workaround for crash in prolonged setup()
    setup();
    setupRun = true;
  }

  // in debugMode 1 the demod output is raw/unparsed so we exit before the parser stage
  static int charsPrinted = 0;
  if (debugMode == 1) {
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

  uint32_t ms = millis();

  //static uint32_t lastUtlityRun = 0;
  //if (!modemTST.adcSpiExclusive && ((ms - lastUtlityRun) >= 100)) {
  //  lastUtlityRun = ms;
  //}

  // begin CID state machine
  switch (AFSKState) {
    case AFSK_READY:
      AFSKReady(state_last, ms);
      break;
    case AFSK_ALTMARKSPACE:
      AFSKAltMark(state_last);
      break;
    case AFSK_ALLMARKS:
      AFSKAllMarks(state_last);
      break;
    case AFSK_DATA:
      AFSKData(state_last);
    case AFSK_CLEANUP:
      AFSKCleanUp(state_last);
      break;
    case AFSK_ERRORx:
      AFSKErrorx(state_last);
      break;
    default:
      AFSKState = AFSK_READY;
  }
}

void printHex(unsigned char* data, int len) 
{
  for (int i = 0; i < len; i++, data++) {
    Serial.print("0x");
    if ((unsigned char)*data <= 0xF) Serial.print("0");
    Serial.print((unsigned char)*data, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void serialize(const char *fname)
{
  File fp = SD.open(fname, FILE_READ);

  unsigned char data[1024];

  for (int i = 0; i < 1023; i++) {
    data[i] = fp.read();
  }

  printHex(data, 1024);
}

void record(I2SSampler *input, const char *fname)
{
  int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
  printf("Start recording");
  input->start();
  // open the file on the sdcard
  File fp = SD.open(fname, FILE_WRITE);
  // create a new wave file writer
  // WAVFileWriter *writer = new WAVFileWriter(fp, input->sample_rate()); //need to replac
  // write out the header - we'll fill in some of the blanks later
  
  // keep writing until the user releases the button
  while ((digitalRead(ONOFFHOOK_PIN) == LOW) && (digitalRead(ONOFFHOOK_WHYS_PIN) == HIGH))
  {
    int samples_read = input->read(samples, 1024);
    int64_t start = esp_timer_get_time();
    // writer->write(samples, samples_read);
    // fwrite(samples, sizeof(int16_t), samples_read, fp);
    fp.write((uint8_t *)samples, sizeof(int16_t) * samples_read);
    int64_t end = esp_timer_get_time();
    printf("Wrote %d samples in %lld microseconds", samples_read, end - start);
    printf("\n");

    if ((digitalRead(ONOFFHOOK_PIN) == HIGH) && (digitalRead(ONOFFHOOK_WHYS_PIN) == LOW)){
      //Serial.println("Begin Hook Flash Timer");
      // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
      // info).
      Timer = timerBegin(0, 80, true);
      // Attach onTimer function to our timer.
      timerAttachInterrupt(Timer, &onTimer, true);
      // Set alarm to call onTimer function every second (value in microseconds).
      // Repeat the alarm (third parameter)
      timerAlarmWrite(Timer, 1000000, true);
      // Start an alarm
      timerAlarmEnable(Timer);

      // hook flash detection
      /* if ((timeCounter < 0.7) && ((digitalRead(ONOFFHOOK_PIN) == LOW) && (digitalRead(ONOFFHOOK_WHYS_PIN) == HIGH))) {
          timerEnd(Timer);
          Timer = NULL;
          timeCounter = 1;
          //Serial.println("Hook flash detected.");
          StaticJsonDocument<48> doc;
          doc["Hook Status"] = "Hook Flash";
          serializeJson(doc, Serial);
      } */
    }
  }
  // stop the input
  printf("\n");
  printf("exited loop\n");
  input->stop();
  // and finish the writing
  // writer->finish(); //need to replace
  // now fill in the header with the correct information and write it again
  printf("before close\n");
  fp.close();
  printf("after close\n");
  // delete writer;
  printf("before free sample\n");
  free(samples);
  printf("Finished recording\n");
  // printf("Finished recording");
}

/************* ONHOOK_NOCALL *************/
/*  State: phone is on hook and is waiting for a call
    1. detects when the ring pin is triggered (phone is ringing)
    2. initial ring suppression
    3. decode CID
    4. if CID matches blacklist, ESP32 receives json packet to play fax tone and do NCID HUP
    5. else, if the ringing times out (~36 s) & phone isn't picked up, next state is ONHOOK_CALLEND
              if not, the next state is OFFHOOK_CALLCONN
    6. if user is making outgoing call or if ESP32 receives json packet to make outgoing call, next state is OFFHOOK_OUTGOING
*/
void onHookNoCall() {
  if (phoneState == ONHOOK_NOCALL) {
    Serial.println("In phone state: ONHOOK_NOCALL");
    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(0.5);
    display.setTextColor(WHITE);
    // Display static text
    display.println("PHONE STATE: On Hook, waiting for action..");
    display.display();
    delay(1000);

    // get ready to suppress first ring on downstream phones
    digitalWrite(FIRSTRING_PIN, HIGH);

    if (digitalRead(RING_PIN) == HIGH) {
      cidSM();
      
      // see if the call number matches blacklist
      if (numberMatchesBL == true) { // cid matches blacklist
        //HUP or fax tone
        for(int i = 0; i < LEDNUM; i++) { // For each pixel...
          // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
          leds.setPixelColor(i, leds.Color(255, 0, 0));
          leds.show();   // Send the updated pixel colors to the hardware.
          //delay(500); // Pause before next pass through loop
        }
      } else {
        // initial ring is suppressed, let the phone continue ringging after a delay
        //delay(1000);
        for(int i = 0; i < LEDNUM; i++) { // For each pixel...
          // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
          leds.setPixelColor(i, leds.Color(0, 255, 0));
          leds.show();   // Send the updated pixel colors to the hardware.
          //delay(500); // Pause before next pass through loop
        }

        digitalWrite(FIRSTRING_PIN, LOW);
        // begin ringing timer
        //Serial.println("Begin RingTimer");
        // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
        // info).
        Timer = timerBegin(0, 80, true);
        // Attach onTimer function to our timer.
        timerAttachInterrupt(Timer, &onTimer, true);
        // Set alarm to call onTimer function every second (value in microseconds).
        // Repeat the alarm (third parameter)
        timerAlarmWrite(Timer, 1000000, true);
        // Start an alarm
        timerAlarmEnable(Timer);

        if ((digitalRead(ONOFFHOOK_PIN) == LOW) && (digitalRead(ONOFFHOOK_WHYS_PIN) == HIGH)) {
          phoneState = OFFHOOK_CALLCONN;
          // report to NCID
        } else if ((timeCounter > 36) && ((digitalRead(ONOFFHOOK_PIN) == HIGH) && (digitalRead(ONOFFHOOK_WHYS_PIN) == LOW))) {
          timerEnd(Timer);
          Timer = NULL;
          timeCounter = 1;
          phoneState = ONHOOK_CALLEND;
        }

      }
    }   
    //Serial.println(digitalRead(ONOFFHOOK_PIN));
    // outgoing call
    if (digitalRead(RING_PIN) == LOW) {
      if ((digitalRead(ONOFFHOOK_PIN) == LOW) && (digitalRead(ONOFFHOOK_WHYS_PIN) == HIGH)) { // OR if .json file is received from gateway to have NCID make call
        // report to ncid
        StaticJsonDocument<48> doc;
        doc["Hook Status"] = "Off Hook";
        serializeJson(doc, Serial);
        phoneState = OFFHOOK_OUTGOING;    
      } else {
        phoneState = ONHOOK_NOCALL;
      }
    }
  }
}

/************* OFFHOOK_CALLCONN *************/
/*  State: call is connected
    1. begin call 1 timer and recording
    2. look for any DTMF key pressed
    3. look for CAS (CPE alerting signal) for CWID
        if CAS detected, get CWID
          if phone is picked up (hook flash), pause call 1 timer and recording, start call 2 timer and recording
          if the user hangs up call 2, start timer and recording for call 2 and go back to call 1 (resume call 1 timer and recording)
          what if there's another call??
    4. if the user hangs up, next state is ONHOOK_CALLEND
    5. if howler tone is detected, tell NCID to alert the user, when on hook is detected, next state is ONHOOK_CALLEND
*/
void offHookCallConn() {
  if (phoneState == OFFHOOK_CALLCONN) {
    Serial.println("In phone state: OFFHOOK_CALLCONN");
    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(0.5);
    display.setTextColor(WHITE);
    // Display static text
    display.println("PHONE STATE: Off Hook, call connected..");
    display.display();
    delay(1000);

    Serial.println("Call connected.");

    // begin call 1 recording and record call status
    StaticJsonDocument<48> doc;
    doc["Call Status"] = "Call Start";
    serializeJson(doc, Serial);
    record(input, "/test.raw");
    
    /* uint8_t tones = dtmf.detect();
    char button = dtmf.tone2char(tones);
    if(button > 0) {
      Serial.print(button);
      Serial.println(" pressed");
    }
    delay(1000); */

    /* if () { // CPE Alerting Signal (CAS) detected
      // get cwid
      cidSM();
      // if call 2 is picked up, pause call 1 recording and timer, start call 2 recording and record call status
      StaticJsonDocument<48> doc;
      doc["Call Status"] = "Call Start";
      serializeJson(doc, Serial);
      record(input, "/test.raw");
    } */

    // if user hangs up
    if ((digitalRead(ONOFFHOOK_PIN) == HIGH) && (digitalRead(ONOFFHOOK_WHYS_PIN) == LOW)) {
      StaticJsonDocument<48> doc;
      doc["Hook Status"] = "On Hook";
      serializeJson(doc, Serial);
      phoneState = ONHOOK_CALLEND;
    } /* else if () { //howler tone is present
      //report to NCID to let user know
      StaticJsonDocument<48> doc;
      doc["Hook Status"] = "On Hook";
      serializeJson(doc, Serial);
      phoneState = ONHOOK_CALLEND;
    } */ else {
      phoneState = OFFHOOK_CALLCONN;
    }
  }
}

/************* OFFHOOK_OUTGOING *************/
/*  State: user is making outgoing call
    1. begin call recording and timer
    2. look for a solid of stutter tone
        if tone was a stutter tone, tell NCID there is a voicemail
        begin dialout timer
        if dtmf is detected within 20 s, next state is OFFHOOK_CALLCONN
          else, next state is ONHOOK_CALLEND
*/
void offHookOutgoing() {
  if (phoneState == OFFHOOK_OUTGOING) {
    Serial.println("In phone state: OFFHOOK_OUTGOING");
    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(0.5);
    display.setTextColor(WHITE);
    // Display static text
    display.println("PHONE STATE: Off Hook, outgoing call..");
    display.display();
    delay(1000);

    // start recording of outgoing call
    StaticJsonDocument<48> doc;
    doc["Call Time"] = "Call Start";
    serializeJson(doc, Serial);
    record(input, "/test.raw");

    /* if () { // solid or stutter tone is detected
      // begin dialout timer
      Serial.println("Begin DialoutTimer");
      // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
      // info).
      dialoutTimer = timerBegin(0, 80, true);
      // Attach onTimer function to our timer.
      timerAttachInterrupt(dialoutTimer, &onDialoutTimer, true);
      // Set alarm to call onTimer function every second (value in microseconds).
      // Repeat the alarm (third parameter)
      timerAlarmWrite(dialoutTimer, 1000000, true);
      // Start an alarm
      timerAlarmEnable(dialoutTimer);

      if (dialoutTimeCounter < 20) { // DTMF detected within ~s
        phoneState = OFFHOOK_CALLCONN;
      } else {
        phoneState = ONHOOK_CALLEND;
      }
    } */
    if ((digitalRead(ONOFFHOOK_PIN) == HIGH) && (digitalRead(ONOFFHOOK_WHYS_PIN) == LOW)) {
      StaticJsonDocument<48> doc;
      doc["Hook Status"] = "On Hook";
      serializeJson(doc, Serial);
      phoneState = ONHOOK_CALLEND;
    }
  }
}

/************* ONHOOK_CALLEND *************/
/*  State: the call has ended, user puts the phone on hook
    1. begin on hook timer
    2. after a few seconds and the phone is on hook, stop on hook and call timer
    3. send recording and call time to database
    4. next state is ONHOOK_NOCALL
*/
void onHookCallEnd() {
  if (phoneState == ONHOOK_CALLEND) {
    Serial.println("In phone state: ONHOOK_CALLEND");
    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(0.5);
    display.setTextColor(WHITE);
    // Display static text
    display.println("PHONE STATE: On Hook, call ended..");
    display.display();
    delay(1000);

    StaticJsonDocument<48> doc;
    doc["Call Status"] = "Call End";
    serializeJson(doc, Serial);

    serialize("/test.raw");

    for(int i = 0; i < LEDNUM; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      leds.setPixelColor(i, leds.Color(0, 0, 0));
      leds.show();   // Send the updated pixel colors to the hardware.
      //delay(500); // Pause before next pass through loop
    }

    phoneState = ONHOOK_NOCALL;
  }
}

/************* AFSK_READY *************/
void AFSKReady(DemodState_t lastAFSKState, uint32_t mstime) {
  if (modemTST.demodState == dmREADY) {
    if (lastAFSKState != dmREADY) {
      if (debugMode > 1) {
        Serial.println(F("\n\n == == == == == == == == "));
        Serial.print(F("\n > dmREADY"));
      }
    }

    // v0.44
    //static uint32_t lastTsScanAtMs = 0;
    //if (!modemTST.adcSpiExclusive && ((millis() - lastTsScanAtMs) > 100)) {
      //AFSK_suspend();
      //handleRingDetector(true);  // v0.72 writes to TFT so suspend demod required.
      //lastTsScanAtMs = mstime;
    //}

    //static uint32_t lastLdrAtMs = 0;
    //const uint32_t ldrInterval = 1000UL;  // x seconds
    //if (((mstime - lastLdrAtMs) > ldrInterval) && !modemTST.adcSpiExclusive) {;
      //lastLdrAtMs = mstime;
    //}

    static uint32_t lastStatsDumpAtMs = 0;
    const uint32_t statsDumpInterval = 30000UL; // 30 seconds
    if ((mstime > statsDumpInterval) && ((mstime - lastStatsDumpAtMs) > statsDumpInterval)) {
      lastStatsDumpAtMs =  mstime;
    }

    lastAFSKState = dmREADY;
  }
}

/************* AFSK_ALTMARKSPACE *************/
void AFSKAltMark(DemodState_t lastAFSKState) {
  if (modemTST.demodState == dmALT_MARK_SPACE) {
    if (lastAFSKState != dmALT_MARK_SPACE) {
      if (debugMode > 1) {
        Serial.print(F("\n > dmALT_MARK_SPACE"));
      }

      //spamFlag = false;  // also cleared in readMcp3002Ch1()
    }

    lastAFSKState = dmALT_MARK_SPACE;
  }
}

/************* AFSK_ALLMARKS *************/
void AFSKAllMarks(DemodState_t lastAFSKState) {
  if (modemTST.demodState == dmALL_MARKS) {
    if (lastAFSKState != dmALL_MARKS) {
      if (debugMode > 1) {
        Serial.print(F("\n > dmALL_MARKS"));
      }
    }

    lastAFSKState = dmALL_MARKS;
  }
}

/************* AFSK_DATA *************/
void AFSKData(DemodState_t lastAFSKState) {
  if (modemTST.demodState == dmDATA) {
    // This section handles reading the demodulator bit queue.
    // Avoid blocking code in this section. No Serial Prints except warning/errors else it may
    // not run fast enough to get all the bits from the demodulator and cause a queue overflow.

    if (lastAFSKState != dmDATA)  {
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
          Serial.println(F("\n > dmCLEANUP"));
        }
      } else if (rcPbq == 2) {
        cli();
        modemTST.demodError = 3;
        modemTST.demodState = dmERRORx;
        sei();
      }
    }

    lastAFSKState =  dmDATA;
  }
}

/************* AFSK_CLEANUP *************/
// get caller id info
void AFSKCleanUp(DemodState_t lastAFSKState) {
  if (modemTST.demodState == dmCLEANUP) {
    // we're at the end of the data stream
    // demod can be suspended from here on without checking.
    AFSK_suspend();
    callsThisRun += 1;

    int mql = fifo_maxQuLen(&modemTST.demodFifo);
    Serial.print(F("max demod Q len: "));
    Serial.print(mql);

    Serial.print(F("\nCHECKSUM  0x"));
    Serial.print(call.checksumCalc, HEX);

    if (call.checksumCalc) {
      Serial.print(F("*FAIL*\n"));
    } else {
      Serial.print(F("**OK**\n"));
      successCountLoop++;
    }

    Serial.print(F("\ndata: \n"));

    // Serial.println(call.date);
    // Serial.println(call.number);
    // Serial.println (call.name);

    StaticJsonDocument<96> doc;
    //char output[128];
    doc["CallerName"] = call.name;
    doc["CallerNumber"] = call.number;
    doc["CallDate"] = call.date;
    //serializeJson(doc, output);
    serializeJsonPretty(doc, Serial);

    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(0.5);
    // Display static text
    display.print("Name: ");
    display.println(call.name);
    display.print("Phone Number: ");
    display.println(call.number);
    display.display();
    delay(1000);

    Serial.print(F("\nOK / total: "));
    Serial.print(successCountLoop);
    Serial.print(" / ");
    Serial.print(callsThisRun);
  }
}

/************* AFSK_ERRORx *************/
void AFSKErrorx(DemodState_t lastAFSKState) {
  if (modemTST.demodState == dmERRORx) {
    AFSK_suspend();

    if (debugMode > 0)
    {
      Serial.println(F("Parser Error"));
      Serial.println(" ");

      Serial.print(F("Error Flag = "));
      Serial.println (modemTST.demodError);

      Serial.print(F("Bad Startbits= "));
      Serial.println(call.CntBadStartBits);

      Serial.print(F("Bad Stopbits= "));
      Serial.println(call.CntBadStopBits);

      Serial.print(F("Parser Errors= "));
      Serial.println(call.CntParserErrors);

      Serial.print(F("Checksum = "));
      Serial.print(call.checkSumStatus);
    }

    cli();
    modemTST.demodState = dmINIT;
    sei();
    lastAFSKState = dmERRORx;
    AFSK_resume();
  }
}

void setup() {
  // open serial connection to output results to serial monitor
  Serial.begin(115200);
  Serial.println(versionNum);

  //configSetup();
  // count system restarts
  //config.runNumber += 1;   
  
  // resave to eeprom
  //eepromDump(); 
  //eepromFetch();
  //PrintEepromVariables();

  // set input pins
  pinMode(ONOFFHOOK_PIN, INPUT);
  pinMode(ONOFFHOOK_WHYS_PIN, INPUT);
  pinMode(CAL_PIN, INPUT);
  pinMode(RING_PIN, INPUT);
  pinMode(ADC1_PIN, INPUT);

  // set output pins
  pinMode(FIRSTRING_PIN, OUTPUT);
  pinMode(DAC1_PIN, OUTPUT);
  pinMode(WSLED_PIN, OUTPUT);

  delay(1000);

  // set up sd card
  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
  }

  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }
  
  // set up displays and wsleds
  // initialize OLED display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  leds.begin();
  leds.setBrightness(50);
  leds.show();
  leds.clear();
  for(int i = 0; i < LEDNUM; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    leds.setPixelColor(i, leds.Color(0, 0, 0));
    leds.show();   // Send the updated pixel colors to the hardware.
    //delay(500); // Pause before next pass through loop
  }

  // setup timer and ADC. From here on, SPI slaves (TS, TFT & ADC) must be interlocked.
  AFSK_init(&modemTST);
  // initial states
  phoneState = ONHOOK_NOCALL;
  // Clear the buffer.
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setCursor(0, 1);
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  // Display static text
  display.println("PHONE STATE: On Hook, waiting for call..");
  display.display();

  AFSKState = AFSK_READY;
  // begin searching for DTMF tones
  adc1_config_width(ADC_WIDTH_BIT_12); // set 12 bit (0-4096)
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); // do not use attenuation
  dtmf.begin((uint8_t)ADC1_PIN); // Use ADC 1, Channel 4 (GPIO32 on Wroom32)

  Serial.println(F("Exiting setup()"));
}

void loop() {
  // phone state machine
  switch (phoneState) {
    case ONHOOK_NOCALL:
      onHookNoCall();
      break;
    case OFFHOOK_CALLCONN:
      offHookCallConn();
      break;
    case OFFHOOK_OUTGOING:
      offHookOutgoing();
      break;
    case ONHOOK_CALLEND:
      onHookCallEnd();
      break;
    default:
      phoneState = ONHOOK_NOCALL;
  }
}