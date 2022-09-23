# 1 "C:\\Users\\jiayi\\AppData\\Local\\Temp\\tmp6rsja700"
#include <Arduino.h>
# 1 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
# 55 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
char versionNr[] = "ESP_callerid_v1.00" ;

#include "Arduino.h"
#include <ESP.h>
#include "Globals.h"
#include "Config.h"
#include "AFSK.h"
#include "XMDF.h"
#include <SPI.h>
#include <ArduinoJson.h>





Afsk modemTST;






byte debugMode = 2 ;
unsigned long callsThisRun = 0 ;
uint32_t successCountLoop = 0 ;
bool dbOnline = false ;
uint32_t dbLastAccessAtMs = 0 ;
bool wlanAvailable = false ;
bool newCallIndicator = false ;
bool spamFlag = false ;
const byte relayPin = 15;

uint32_t resourceMonitorSuspendedAtMs = 0 ;
bool displayShowsLatestCall = false ;



void tmConfMain_root( byte par ) ;
void tmGetDebugMode_cbr( byte par ) ;
void fNetworkMode_cbr( byte par ) ;
void tmViewConfigPage2( byte par ) ;
void tmConfMain_cbr ( byte par ) ;
void getCallFromDb( byte par ) ;
void navBarReturnToConfRoot( void ) ;
void tmRestart( byte par ) ;
void handleRingDetector( bool force ) ;
# 208 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
void ICACHE_FLASH_ATTR tmConfMain_root ( byte par );
void ICACHE_FLASH_ATTR handleRingDetector( bool force );
void ICACHE_FLASH_ATTR setup();
void ICACHE_FLASH_ATTR setup1();
void loop();
#line 208 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
void ICACHE_FLASH_ATTR tmConfMain_root ( byte par ) {




  swSerial.println(F("tmConfMain_root()"));

  newCallIndicator = false ;
  displayShowsLatestCall = false ;

  AFSK_suspend() ;
# 240 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
}
# 847 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
void ICACHE_FLASH_ATTR handleRingDetector( bool force ) {
# 856 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
  const bool localDebug = true ;

  uint32_t ms = millis() ;

  typedef enum {
    RINGING = 1,
    NEWCALL,
    BLANKOVER,
  } displayControl_t ;

  displayControl_t displayNow ;
  static displayControl_t lastDisplayed = BLANKOVER;

  if ( inShortRingBurst ) displayNow = RINGING ;
  else if ( newCallIndicator ) displayNow = NEWCALL ;
  else displayNow = BLANKOVER ;


  static uint32_t lastDisplayUpdateAtMs = 0 ;
# 907 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
  static bool inShortRingBurstLast = false ;
  if ( inShortRingBurstLast != inShortRingBurst ) {
    if ( localDebug ) {
      swSerial.print( "inShortRingBurst=" ) ;
      swSerial.println( inShortRingBurst ) ;
    }
    inShortRingBurstLast = inShortRingBurst ;
  }

  static bool inLongRingBurstLast = false ;
  if ( inLongRingBurstLast != inLongRingBurst ) {
    if ( localDebug ) {
      swSerial.print( "inLongRingBurst=" ) ;
      swSerial.println( inLongRingBurst ) ;
    }
    if ( inLongRingBurst ) {


      spamFlag = false ;
    }
    else {


      digitalWrite( relayPin, LOW ) ;
    }
    inLongRingBurstLast = inLongRingBurst ;
  }

  if ( inLongRingBurst && spamFlag ) {
    digitalWrite( relayPin, HIGH ) ;
  }
}
# 954 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
void ICACHE_FLASH_ATTR setup() {

}


void ICACHE_FLASH_ATTR setup1() {




  swSerial.begin(115200);
  swSerial.println( "version: " ) ;
  swSerial.println( versionNr ) ;

  SPI.begin( );

  pinMode( relayPin, OUTPUT ) ;
  digitalWrite( relayPin, LOW ) ;


  configSetup() ;
  config.runNumber += 1 ;

  eepromDump() ;
  eepromFetch() ;
  PrintEepromVariables() ;
# 1010 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
  delay ( 1000 ) ;






  delay ( 1000 ) ;
# 1032 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
  delay( 10000 ) ;



  AFSK_init( &modemTST ) ;

  swSerial.print( F("\nexiting setup()" )) ;

}






void loop() {

  static DemodState_t state_last ;
  static bool setupRun = false ;

  if ( ! setupRun ) {

    setup1() ;
    setupRun = true ;
  }





  static int charsPrinted = 0 ;
  if ( debugMode == 1 )
  {
    static bool runOnce = false ;
    if ( ! runOnce ) {
      runOnce = true ;
      AFSK_resume() ;
    }
    if ( modemTST.curr_bitReady == true ) {
      cli() ;
      modemTST.curr_bitReady = false ;
      sei() ;
      swSerial.print( modemTST.curr_bit ) ;
      charsPrinted ++ ;
      if ( charsPrinted == 80 ) {
        charsPrinted = 0 ;
        swSerial.println() ;
      }

    }
    return ;
  }


  yield() ;

  uint32_t ms = millis() ;

  static uint32_t lastUtlityRun = 0 ;
  if ( ! modemTST.adcSpiExclusive && ms - lastUtlityRun >= 100 ) {
    lastUtlityRun = ms ;



  }
# 1109 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
  if ( modemTST.demodState == dmREADY ) {

    if ( state_last != dmREADY ) {
      if ( debugMode > 1 ) {
        swSerial.println( F("\n\n == == == == == == == == ") ) ;
        swSerial.print( F("\n > dmREADY") ) ;
      }
    }



    static uint32_t lastTsScanAtMs = 0 ;
# 1135 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
    static uint32_t lastLdrAtMs = 0 ;
    const uint32_t ldrInterval = 1000UL ;
    if ( ms - lastLdrAtMs > ldrInterval && ! modemTST.adcSpiExclusive ) {


      lastLdrAtMs = ms ;
    }


    static uint32_t lastStatsDumpAtMs = 0 ;
    const uint32_t statsDumpInterval = 30000UL ;
    if ( ms > statsDumpInterval && ms - lastStatsDumpAtMs > statsDumpInterval ) {

      lastStatsDumpAtMs = ms ;
    }


    state_last = dmREADY ;
  }






  else if ( modemTST.demodState == dmALT_MARK_SPACE ) {

    if ( state_last != dmALT_MARK_SPACE ) {

      if ( debugMode > 1 ) swSerial.print( F("\n > dmALT_MARK_SPACE") ) ;

      spamFlag = false ;
      displayShowsLatestCall = false ;

    }
    state_last = dmALT_MARK_SPACE ;
  }






  else if ( modemTST.demodState == dmALL_MARKS ) {

    if ( state_last != dmALL_MARKS ) {

      if ( debugMode > 1 ) swSerial.print( F("\n > dmALL_MARKS") ) ;

    }
    state_last = dmALL_MARKS;
  }





  else if ( modemTST.demodState == dmDATA ) {







    if ( state_last != dmDATA ) {

      for ( byte i = 0 ; i < sizeof ( call.allBytes ) ; i++ ) {
        call.allBytes[i] = 0 ;
      }
      if ( debugMode > 1 ) swSerial.print( F("\n > dmDATA") ) ;
    }

    if ( ! fifo_isempty_locked( &modemTST.demodFifo ) ) {

      bool inBit = fifo_pop_locked( &modemTST.demodFifo ) ;

      byte rcPbq = parseDemodBitQueue( inBit ) ;

      if ( rcPbq == 1 ) {
        cli() ;
        modemTST.demodState = dmCLEANUP ;
        sei() ;
        if ( debugMode > 1 ) {
          swSerial.println( F( "\n > dmCLEANUP" ) ) ;
        }
      }
      else if ( rcPbq == 2 ) {
        cli() ;
        modemTST.demodError = 3 ;
        modemTST.demodState = dmERRORx ;
        sei() ;
      }

    }
    state_last = dmDATA ;
  }





  else if ( modemTST.demodState == dmCLEANUP ) {




    AFSK_suspend() ;
    callsThisRun += 1 ;

    int mql = fifo_maxQuLen( &modemTST.demodFifo ) ;
    swSerial.print(F("max demod Q len: ")) ;
    swSerial.print( mql ) ;

    swSerial.print( F("\nCHECKSUM  0x" )) ;
    swSerial.print( call.checksumCalc , HEX ) ;
    if ( call.checksumCalc ) {
      swSerial.print( F(" *FAIL*\n" )) ;
    }
    else {
      swSerial.print( F(" **OK**\n" ) ) ;
      successCountLoop ++ ;
    }

    swSerial.print ( F("\ndata: \n" ) ) ;

    swSerial.println ( call.date ) ;
    swSerial.println ( call.number ) ;
    swSerial.println ( call.name ) ;

    swSerial.print( F( "\nOK / total: ") ) ;
    swSerial.print( successCountLoop ) ;
    swSerial.print( " / " ) ;
    swSerial.print( callsThisRun ) ;

    newCallIndicator = true ;
# 1294 "C:/Users/jiayi/OneDrive/Documents/PlatformIO/Projects/220911-233457-esp32doit-devkit-v1/src/ESP_Telephone_Callerid_v1.00_public.ino"
    cli() ;
    modemTST.demodState = dmINIT ;
    sei() ;
    state_last = dmCLEANUP ;
    AFSK_resume() ;
  }





  else if ( modemTST.demodState == dmERRORx ) {
    AFSK_suspend() ;
    if ( debugMode > 0 )
    {

      swSerial.println(F(" Parser Error")) ;
      swSerial.println(" ") ;

      swSerial.print(F("Error Flag = " )) ;
      swSerial.println (modemTST.demodError ) ;

      swSerial.print( F("Bad Startbits= " )) ;
      swSerial.println( call.CntBadStartBits ) ;

      swSerial.print( F("Bad Stopbits= " ) ) ;
      swSerial.println( call.CntBadStopBits ) ;

      swSerial.print( F("Parser Errors= " ) ) ;
      swSerial.println( call.CntParserErrors ) ;

      swSerial.print( F("Checksum = " )) ;
      swSerial.print( call.checkSumStatus ) ;

    }


    cli();
    modemTST.demodState = dmINIT ;
    sei();
    state_last = dmERRORx ;
    AFSK_resume() ;
  }
}