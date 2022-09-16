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
#define FIRSTRING_PIN 34

// define states
#define ONHOOK_NOCALL 1
#define OFFHOOK_CALLCONN 2
#define OFFHOOK_OUTGOING 3
#define ONHOOK_CALLEND 4


// global variables
//static unsigned long timeCounter;
static unsigned int state;
static unsigned int ring;
static unsigned int onoff_hook;

char NameOfPerson[];
char PhoneNumber[];
static unsigned int TimeOfCall;


// prototype functions
/* void IRAM_ATTR onTimer();
void timerFunction(); */
void createJson();
void state1();
void state2();
void state3();
void state4();

// a pointer variable timer of the type hw_timer_t in order to configure the timer
/* hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
  timeCounter++;
  Serial.println(String("Seconds passed ") + String(timeCounter));
}

void timerFunction() {
  Serial.println("Start timer ");
  // timer 0, CP = 1 ns, count up
  timer = timerBegin(0, 80, true);
  // edge triggered
  timerAttachInterrupt(timer, &onTimer, true);
  // 1 s
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
} */

void state1() {
  if (ring == HIGH) {
        // suppress 1st first ring
        // need another state machine with CID?
        //    1. channel seizure tones to mark
        //    2. AFSK demodulataor
        //    3. decode cid packet
        //    4. send info to NCID
        // if matches with NCID blacklist,
        //    HUP (send a fax tone packet through pin D25)
        // else
        //    let the phone keep ringing
        //    start timer for the ringtime
        //    if ringtime times out
        //        state = ONHOOK_CALLEND
        if ((ONOFFHOOK_PIN == HIGH) || (ONOFFHOOK_WHYS_PIN == HIGH)) {
          state = OFFHOOK_CALLCONN;
          // report to NCID
        }
  }
  else {
    if ((ONOFFHOOK_PIN == HIGH) || (ONOFFHOOK_WHYS_PIN == HIGH)) { // OR if .json file is received from gateway to have NCID make call
      state = OFFHOOK_OUTGOING;
      // report to NCID
    }
  }
}

void state2() {
  // begin call 1 recording and timer
  // if DTMF is detected
    // if DTMF is keypad
      //check what numbers are being pressed
    // if DTMF is user or caller hanging up
      state = ONHOOK_CALLEND;
    // if Howler tone is detected
      // report to NCID & let user know
      state = ONHOOK_CALLEND;
}

void state3() {
  // if DTMF solid tone or stutter tone is detected
    // start another timer for user or NCID dialout
      // if keypad DTMF isnt detected within ~s
        state = ONHOOK_CALLEND;
      // if ringtone DTMF is detected
        state = OFFHOOK_CALLCONN;
      // if tone is a stutter
        // send notice to gateway driver about VM
}

void state4() {
  // if phone has been on hook for ~ms
    // stop call ercording and timer
    // report end of call time and other info to ncid
    // second recording to database
    state = ONHOOK_NOCALL;
}

void setup() {

  // open serial connection to output results to serial monitor
  Serial.begin(115200);

  // set input pins
  pinMode(ONOFFHOOK_PIN, INPUT);
  pinMode(ONOFFHOOK_WHYS_PIN, INPUT);
  pinMode(CAL_PIN, INPUT);
  pinMode(RING_PIN, INPUT);

  // set output pins
  pinMode(FIRSTRING_PIN, OUTPUT);
  pinMode(DAC1_PIN, OUTPUT);

  state = ONHOOK_NOCALL;

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
  
  // state machine
  switch (state) {
    case ONHOOK_NOCALL:
      state1();
      break;
    case OFFHOOK_CALLCONN:
      state2();
      break;
    case OFFHOOK_OUTGOING:
      state3();
      break;
    case ONHOOK_CALLEND:
      state4();
      break;
    default:
      state = ONHOOK_NOCALL;
  }

}