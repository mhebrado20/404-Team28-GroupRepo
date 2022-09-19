/*
    Telephone Caller ID with anti-spam option for ESP8266

    Tested with ESP8266 Arduino core 2.4.0 / Adafruit Huzzah Feather.

    Software / document location: http://forum.arduino.cc/index.php?topic=528459.0
    
    6v6gt 11.02.2018


 

    Usage Notes
    
    - You are advised to set at least the wlan credentials in the "factory settings" of
       Config.cpp before uploading the software to the device for the first time.
       This option is available only on the first upload, otherwise use the AP mode method mentioned below.
    - For the first configuration, ensure to accept the option to configure the touch screen
       otherwise the buttons dont work. The buttons are unresonsive for the first up to 10 seconds
       after first appearing after a system start.
       In the default debug mode, you see a small circle when touch the screen, which may help
       to get an accurate hit on the menu buttons.
    - Do not attempt to put the device into online mode (Menu point 2.) until the wlan credentials 
       have been successfully entered. A password (psk) is mandatory for your wlan.
    - Put the into AP mode  (menu point 3. Set Online Conf (AP) to enter the credentials by
       connecting to SSID: CallerId and IP address: 192.168.1.4  (timeout: 5 minutes).
       You can just enter your local wlan credentials, then you can access the device through
       your wlan for the remainder of the configuration.    
    - You can see the IP address that the device has been assigned in your wlan through the 
       configuration menu ( menu point 4. page 2).   
    - The configuration mode is not stored and any changes are valid only for the current session.   


    Pin Usage

    SPI CS Pins:
    4  TFT_CS_PIN
    16 TS_CS_PIN
    2  ADC_CS_PIN

    SPI Bus
    14  SPI SCK
    13  SPI MOSI
    12  SPI MISO

    5   TFT_DC_PIN  (Screen Data Control)
    0   pwmPin (Backlight control)
    15  relayPin

    A0  (ldr)

*/


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

/* NavBar tmNavBar( NAVBARBOTTOM ) ;
NavBar tmReturnToConfigRoot( NAVBARBOTTOM ) ;  // does not change so merits a separate instance */


// references in Globals.h run control and stats
byte debugMode = 2 ;  // updated in setup()   
unsigned long callsThisRun = 0 ;
uint32_t successCountLoop = 0 ;
/* bool dbOnline = false ;                     // (!client.connect( config.remoteHost , 80 ))  tested / set periodically. if false for x seconds, restart forced.
uint32_t dbLastAccessAtMs = 0 ;             // last successful access to DB. Set in get getCallFromDb(). Used by resourceMonitor().
bool wlanAvailable = false  ;               // WiFi.status() != WL_CONNECTED
 */
bool newCallIndicator = false ;
bool spamFlag = false ;                     // used in getCallFromDb() && handleRingDetector() && loop()
const byte relayPin = 15;

/* uint32_t resourceMonitorSuspendedAtMs = 0 ;
bool displayShowsLatestCall = false  ;  // set in getCallFromDb, unset when entering config,    tested in resourceMonitor() ;
 */

// prototypes

/* void tmConfMain_root( byte par ) ;
void tmGetDebugMode_cbr( byte par ) ;
void fNetworkMode_cbr( byte par ) ;
void tmViewConfigPage2( byte par ) ;
void tmConfMain_cbr ( byte par ) ;
void getCallFromDb( byte par ) ;
void navBarReturnToConfRoot( void ) ;
void tmRestart( byte par ) ; */
void handleRingDetector( bool force ) ;



// don't need
/* void ICACHE_FLASH_ATTR displayNewCall() {
  //
  // offline mode format
  //
  swSerial.print(F("in displayNewCall() "));

  displayShowsLatestCall = true ;

  tft.fillScreen( ILI9341_BLACK );

  tft.setCursor( 0, 10 );   // x, y
  tft.setTextSize(4);

  if ( strlen( call.number ) > 0 )  tft.println ( call.number ) ;
  else tft.println ( "No Number" ) ;

  tft.setTextSize(1);  // was 2
  tft.println( ) ;

  tft.setTextSize(3);
  tft.println ( call.name ) ;

  tft.setTextSize(1);  // was 2
  tft.println( ) ;

  tft.setTextSize(3);
  tft.println ( call.date ) ;


  tft.setTextSize(1);
  tft.println( ) ;

  tft.setTextSize(2);

  tft.print( call.checkSumStatus ) ;
  //  tft.println( ) ;
  tft.print( F( "  OK / tot: ") ) ;
  tft.print( successCountLoop ) ;
  tft.print( " / " ) ;
  tft.print( callsThisRun ) ;
  tft.println( ) ;

  tft.setTextSize(1);
  tft.println( ) ;
  tft.setTextSize(2);
  tft.print(  (wlanAvailable ? "WLAN  " : "No Net  " ) ) ;
  tft.print(  ( newCallIndicator ? "*new call*" : " "  )  ) ;
  tft.println( ) ;

  tmNavBar.UpdateNavBarItem(   "  configure" , tmConfMain_root , 1, true ) ;
  if ( config.networkMode  ) {
    tmNavBar.UpdateNavBarItem(   "    more" ,  getCallFromDb , 0, true ) ;
  }
  else {
    tmNavBar.UpdateNavBarItem(   "    more" , NULL , 2, false ) ;
  }

  tmNavBar.Show(); //
  swSerial.print(F("exiting displayNewCall() "));
}
 */

// don't need
/* void  ICACHE_FLASH_ATTR displayNewCallError() {

  swSerial.print(F("in displayNewCallError() "));
  displayShowsLatestCall = false ;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.println(F(" Parser Error")) ;
  tft.println(" ") ;

  // could be better structured.
  tft.print(F("Error Flag = " )) ;
  tft.println (modemTST.demodError ) ;
  tft.println() ;

  tft.print( F("Bad Startbits= " )) ;
  tft.println( call.CntBadStartBits ) ;

  tft.print(  F("Bad Stopbits= " ) ) ;
  tft.println( call.CntBadStopBits ) ;

  tft.print( F("Parser Errors= " ) ) ;
  tft.println( call.CntParserErrors ) ;

  tft.print( F("Checksum = " )) ;
  tft.print( call.checkSumStatus ) ;

  tmNavBar.UpdateNavBarItem(   "  configure" , tmConfMain_root , 1, true ) ;
  if ( config.networkMode  ) {
    tmNavBar.UpdateNavBarItem(   "    more" ,  getCallFromDb , 0, true ) ;
  }
  else {
    tmNavBar.UpdateNavBarItem(   " " , NULL , 2, false ) ;
  }
  tmNavBar.Show(); //
}
 */



// don't need
/* void  ICACHE_FLASH_ATTR tmConfMain_root ( byte par ) {
  //
  // par unused
  //

  swSerial.println(F("tmConfMain_root()"));

  newCallIndicator = false ; // clear
  displayShowsLatestCall = false ;

  AFSK_suspend() ;  // suspend demodulator - resumed on user exit or timeout.
  NavBar::navBarSessionActive = true ;

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.println(F(" Main Config Menu")) ;
  tft.println(" ") ;
  tft.println(F(" 1. Set Debug Options")) ;
  tft.println(F(" 2. Set Online/Offline")) ;
  tft.println(F(" 3. Set Online Conf (AP)")) ;
  tft.println(F(" 4. View Conf")) ;
  tft.println(F(" 5. Restart")) ;
  tft.println(F(" 6. Exit")) ;

  tmNavBar.UpdateNavBarItem(  " 1" , tmConfMain_cbr , 1, true ) ;
  tmNavBar.UpdateNavBarItem(  " 2" , tmConfMain_cbr , 2, true ) ;
  tmNavBar.UpdateNavBarItem(  " 3" , tmConfMain_cbr , 3, true ) ;
  tmNavBar.UpdateNavBarItem(  " 4" , tmConfMain_cbr , 4, true ) ;
  tmNavBar.UpdateNavBarItem(  " 5" , tmConfMain_cbr , 5, true ) ;
  tmNavBar.UpdateNavBarItem(  " 6" , tmConfMain_cbr , 6, true ) ;
  tmNavBar.Show() ;
}
 */

// don't need
/* void ICACHE_FLASH_ATTR tmConfMain_cbr ( byte par ) {
  //
  // par 1-5 for menu items 1-5
  //

  if ( ! modemTST.adcSpiExclusive  ) {
    swSerial.print(F("in tmConfMain_cbr() par= "));  swSerial.print( par ) ;


    switch ( par ) {
      case 1 :  // debug options
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 20);
        tft.setTextSize(2);
        tft.println(F(" Select Debug Option")) ;
        tft.println(" " ) ;
        tft.println(" " ) ;
        tft.println(F(" Default=2")) ;
        tft.print(F(" Current=")) ;
        tft.println( debugMode   ) ;

        if ( debugMode != 0 ) tmNavBar.UpdateNavBarItem(   "  0" , tmGetDebugMode_cbr , 0, true ) ;
        else  tmNavBar.UpdateNavBarItem(   "  0" , NULL , 0, false ) ;

        if ( debugMode != 1 ) tmNavBar.UpdateNavBarItem( "  1" , tmGetDebugMode_cbr , 1, true ) ;
        else tmNavBar.UpdateNavBarItem(   "  1" , NULL , 0, false ) ;

        if ( debugMode != 2 )  tmNavBar.UpdateNavBarItem( "  2" , tmGetDebugMode_cbr , 2, true ) ;
        else tmNavBar.UpdateNavBarItem(   "  2" , NULL , 0, false ) ;

        if ( debugMode != 3 )  tmNavBar.UpdateNavBarItem( "  3" , tmGetDebugMode_cbr , 3, true ) ;
        else tmNavBar.UpdateNavBarItem(   "  3" , NULL , 0, false ) ;

        tmNavBar.UpdateNavBarItem( "Ret" , tmConfMain_root    , 0, true ) ;
        tmNavBar.Show() ;
        break ;


      case 2 :  // online/offline
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 20);
        tft.setTextSize(2);
        tft.println(F(" Set Online/Offline")) ;
        tft.println(" ") ;
        tft.println(" " ) ;
        tft.println(F(" Current Network Mode=")) ;
        tft.println(" ") ;
        tft.println( config.networkMode ?  F(" Online") : F(" Offline")   ) ;
        tft.println(" " ) ;
        tft.println(" Press to change:" ) ;


        // tmNavBar.Clear() ;
        //
        tmNavBar.UpdateNavBarItem( "Offline" ,      fNetworkMode_cbr ,  0, config.networkMode ) ;
        tmNavBar.UpdateNavBarItem( "Online" ,       fNetworkMode_cbr ,  1, ! config.networkMode ) ;
        tmNavBar.UpdateNavBarItem( "Return" ,        tmConfMain_root ,  2, true ) ;
        tmNavBar.Show() ;

        break ;


      case 3 :  // AP mode

        swSerial.println("Option 3 AP mode set up ") ;

        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 20);
        tft.setTextSize(2);
        tft.println(F(" Set Online Config")) ;
        tft.println(" ") ;

        resourceMonitorSuspendedAtMs = millis() ;
        setupAP() ;

        tft.print(F(" SSID= ")) ;
        tft.println( apName   ) ;

        delay( 1000 ) ;  // enough ? else force to a new menu with a refresh button

        WebServerSetup() ;  

        tft.print(F(" IP SoftAP= ")) ;
        tft.println( WiFi.softAPIP()   ) ;

        tft.println(F(" Do sytem Restart")) ;
        tft.println(F("   when finished")) ;

        tmNavBar.UpdateNavBarItem( " " , NULL, 2, false ) ;
        tmNavBar.UpdateNavBarItem( " " , NULL, 2, false ) ;
        tmNavBar.UpdateNavBarItem( " restart " , tmRestart , 0, true ) ;
        tmNavBar.Show() ;

        break;


      case 4 :  // view Conf

        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 20);
        tft.setTextSize(2);
        tft.println(F(" View Configuration")) ;
        tft.println(" ") ;

        tft.print(F(" ")) ;
        tft.println( versionNr   ) ;

        tft.print(F(" Debug Mode= ")) ;
        tft.println( debugMode   ) ;

        tft.print(F(" Network Mode cfg= ")) ;
        tft.println( config.networkMode ?  F("Online") : F("Offline")   ) ;

        tft.print(F(" run number= ")) ;
        tft.println( config.runNumber  ) ;

        tft.print(F(" calls this run= ")) ;
        tft.println( callsThisRun  ) ;

        tft.print(F(" newCallIndicator= ")) ;
        tft.println( newCallIndicator ?  F("True") : F("False")   ) ;

        tmNavBar.UpdateNavBarItem( " " , NULL, 2, false ) ;
        tmNavBar.UpdateNavBarItem( " " , NULL, 2, false ) ;
        tmNavBar.UpdateNavBarItem( " next" , tmViewConfigPage2 , 2, true ) ;
        tmNavBar.Show() ;

        break ;

      case 5 :  // Restart
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 20);
        tft.setTextSize(2);
        tft.println(F(" Forcing restart")) ;
        tft.println(F(" Please wait ... ")) ;
        while ( true ) ; // wdt forces off !
        break ;


      case 6 :  // Exit

        if ( config.networkMode ) getCallFromDb( 0 ) ; //  latest call (if any) / db format
        else displayNewCall( ) ; //  current call (if any) / offline format

        // we have exited config. Free and restart demodulater
        NavBar::navBarSessionActive = false ;
        AFSK_resume() ;
        break ;
    }
  }
}
 */

// don't need
/* void ICACHE_FLASH_ATTR tmViewConfigPage2( byte par ) {

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.println(F(" View Configuration")) ;
  tft.println(" ") ;

  tft.print(F(" IP station= ")) ;
  tft.println( WiFi.localIP()  ) ;

  tft.print(F(" IP SoftAP= ")) ;
  tft.println( WiFi.softAPIP()   ) ;


  tft.print(F(" wlanAvailable= ")) ;
  tft.println( wlanAvailable ?  F("True") : F("False")   ) ;

  tft.print(F(" dbOnline= ")) ;
  tft.println( dbOnline ?  F("True") : F("False")   ) ;

  tft.print(F(" spam level (0-3)= ")) ;
  tft.println( config.spamLevel ) ;

  tmReturnToConfigRoot.Show() ;
}
 */

// don't need
/* void ICACHE_FLASH_ATTR tmGetDebugMode_cbr( byte mode ) {
  //
  //  mode contains new debugMode
  //

  swSerial.println(F("in tmGetDebugMode_cbr() "));
  debugMode = mode ;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.println(" ") ;
  tft.println(" ") ;
  tft.print(F("setting debug="));
  tft.println( debugMode );

  tmReturnToConfigRoot.Show() ;
}
 */

// don't need
/* void  ICACHE_FLASH_ATTR  fNetworkMode_cbr(  byte par ) {
  //
  // par:  0=standalone, 1=network
  //
  // change handled - not tidy. Cleaner possible to automatically force a restart if change made.
  //

  swSerial.print(F("in fNetworkMode_cbr() par= "));  swSerial.print( par ) ;

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);

  if ( par == 0 || par == 1 ) {
    if ( par != config.networkMode ) {

      config.networkMode = ( par == 1 ) ;
      tft.println(F(" New Network Mode= ")) ;
      tft.println( config.networkMode ?  F("  Online") : F("  Offline")   ) ;
      tft.println(" ") ;
      // tft.println(F(" Restart system" )) ;
      // tft.println(F("   to activate any changes")) ;

      eepromDump() ; // resave to eeprom
      eepromFetch() ;
      PrintEepromVariables() ;

      // wlan
      if ( config.networkMode ) {
        // generates messages so wait:
        delay( 1000 ) ;
        setupSTA( false ) ;
      }
      else {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
      }
    }
    else {
      tft.println(F(" Network Mode unchanged=")) ;
      tft.println( config.networkMode ?  F("  Online") : F("  Offline")   ) ;
    }
  }
  tmReturnToConfigRoot.Show() ;
}
 */

// don't need
void ICACHE_FLASH_ATTR tmRestart( byte mode ) {
  //
  //  mode contains 0
  //

  swSerial.println(F("in tmRestart() "));
  swSerial.println(F("Forcing reset using WDT. )lease wait . . . "));
  while ( true ) ; // use WDT !!

}



// don't need
/* void getCallFromDb( byte par ) {
  //
  // gets call based on par from db and displays it as a history record
  //
  // par == 0: get latest;     // called via setup and loop to force fetc of latest record.
  // par == 1 get previous ;   // called via button
  // par == 2 get next;        // called via button
  // protect against overshoot.
  // Does not handle gaps in db row sequence numbers (id)
  //

  swSerial.print(F("\nin getCallFromDb() par= ")); swSerial.println( par );

  static uint32_t  maxIdFound = 0 ;  // since last fetch of latest record
  static uint32_t  currentId = 0  ;  // working record
  bool isErrorTooLow = false ;
  bool isErrorTooHigh = false ;

  displayShowsLatestCall = false ;   // assume not

  if ( par != 0 ) newCallIndicator = false ; // clear

  if ( par == 0  || ( par != 0 && ( maxIdFound == 0  || currentId == 0 )  )   ) {
    // force a fetch of latest record
    maxIdFound = 0 ;
    currentId = 0 ;
  }
  else if ( par == 1 ) {
    // back
    if ( currentId > 1 )  currentId -- ;  // step back 1
    else isErrorTooLow = true ;
  }
  else if ( par == 2 ) {
    // back
    if ( currentId < maxIdFound )  currentId ++ ; // step forward 1
    else isErrorTooHigh = true ;
  }


  WiFiClient client;
  swSerial.print(F("connecting to "));
  swSerial.println( config.remoteHost );

  if (!client.connect( config.remoteHost , 80 )) {
    swSerial.println(F("connection failed"));
    dbOnline = false ;
  }
  else {

    // We now create a URI for the request

    String url = String( config.phpRetrievePath )  + String( "?id=" ) + String( currentId  ) ;
    swSerial.print(F("Requesting URL: "));
    swSerial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + config.remoteHost + "\r\n" +
                 "Connection: close\r\n\r\n");


    swSerial.println(F("Skip to beginning of html"));
    char endOfHeaders[] = "***Json:";
    if ( ! client.find(endOfHeaders) ) {
      swSerial.println(F("Json marker not found"));
      dbOnline = false ;
    }
    else {

      swSerial.println(F("Allocate JsonBuffer"));
      // ver 0.44a
      const size_t BUFFER_SIZE = 440 ;  // v0.71 was 512
      DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);

      swSerial.println(F("Parse JSON object"));

      // jsonBuffer.clear() ;

      // Jsonlib has problems with NULLs - investigate v0.42
      JsonObject& root = jsonBuffer.parseObject(client);
      if (root.success()) {

        swSerial.println(F("Extract values:"));
        swSerial.println(root["ErrorCode"].as<char*>());
        swSerial.println(root["ErrorText"].as<char*>());

        if ( strcmp(  ( root["ErrorCode"].as<char*>() ) , "0" ) == 0 ) {

          if ( par == 0 ) {
            // set / reset if we are getting the latest
            maxIdFound = atol(root["id"].as<char*>()) ;
            currentId  = maxIdFound ;
          }

          swSerial.print(F("\nmaxIdFound: ") );
          swSerial.println( maxIdFound );


          swSerial.println(root["id"].as<char*>());
          swSerial.println(root["timeStampSystem"].as<char*>());
          swSerial.println(root["telNo"].as<char*>());
          swSerial.println(root["nameNet"].as<char*>());
          swSerial.println(root["dateStampNet"].as<char*>());
          swSerial.println(root["checkDigitOk"].as<char*>());
          swSerial.println(root["numberInRun"].as<char*>());
          swSerial.println(root["runNumber"].as<char*>());
          swSerial.println(root["name"].as<char*>());

          swSerial.println();

          swSerial.println(F("Preparing to write to tft:"));

          dbOnline = true ;


          // calculate spamFlag   v0.69

          
          //  0. spam filter off
          //  1. reject suppressed number
          //  2. reject no network name
          //  3. reject all except white list numbers
          

          if ( config.spamLevel == '0' ) spamFlag = false ;
          else if ( strlen(root["name"].as<char*>() ) > 0 )   spamFlag = false ;  // in white list with name
          else if ( strlen(root["name"].as<char*>() ) == 0  && config.spamLevel == '3' ) spamFlag = true ;  // not in whitelist
          else if ( strlen(root["telNo"].as<char*>() ) == 0 && config.spamLevel >= '1' ) spamFlag = true ;  // suppressed number
          else if ( strlen(root["nameNet"].as<char*>() ) == 0 && config.spamLevel >= '2' )   spamFlag = true ;  // no network name
          else spamFlag = false ;


          tft.fillScreen( ILI9341_BLACK );

          tft.setCursor( 0, 10 );   // x, y
          tft.setTextSize(4);

          String telNoTrunc = root["telNo"].as<char*>() ;

          if ( spamFlag ) tft.setTextColor( ILI9341_RED   ) ;

          if ( telNoTrunc.length() > 12 )  tft.println ( telNoTrunc.substring( 0 , 12 ) + ">" ) ;
          else if ( telNoTrunc.length() == 0  ) tft.println ( "No Number") ;
          else tft.println ( telNoTrunc  ) ;

          if ( spamFlag ) tft.setTextColor( ILI9341_WHITE   ) ;  // restore default colour


          tft.setTextSize(2);
          tft.println( ) ;

          tft.setTextSize(3);
          if ( strlen(  root["name"].as<char*>()  ) > 0 ) {
            tft.println ( root["name"].as<char*>() ) ;
          }
          else if ( strlen(  root["nameNet"].as<char*>()  ) > 0 ) {
            tft.println ( root["nameNet"].as<char*>() ) ;
          }
          else tft.println (F( "UNKNOWN" )) ;

          tft.setTextSize(2);
          tft.println( ) ;

          tft.setTextSize(2);
          tft.println ( root["timeStampSystem"].as<char*>() ) ;
          tft.println( ) ;

          if ( strlen(  root["name"].as<char*>()  ) > 0 ) tft.println ( root["nameNet"].as<char*>() ) ;       // maybe blank
          else tft.println ( " " ) ;

          tft.println( ) ;

          String statusLine = "" ;

          if ( strcmp( root["checkDigitOk"].as<char*>()  , "1" ) == 0 )   statusLine += "(OK) " ;
          else statusLine += "(NOK)" ;

          if ( dbOnline && wlanAvailable ) statusLine += "   online " ;
          else  statusLine += "   offline" ;

          if ( newCallIndicator ) statusLine += " *new call*" ;

          tft.println( statusLine ) ;

          tft.println( ) ;

          // v0.74 we do this as late as posible:
          if ( par == 0 || currentId  == maxIdFound ) {
            swSerial.println(F( "we've got the latest call" )) ;
            displayShowsLatestCall = true ;
          }


        }
        else {
          // error found
          swSerial.println(F("Error found"));
          dbOnline = false ;
          dbOnline =  false ;
          tft.fillScreen( ILI9341_BLACK );
          tft.setCursor( 0, 10 );   // x, y
          tft.setTextSize(3);
          tft.println (F( "not found" )) ;
        }

      }
      else {
        swSerial.println (F("Parsing failed!"));
        dbOnline = false ;
      }
    }
  }
  delay( 10 ) ;

  if ( dbOnline ) {
    dbLastAccessAtMs = millis() ;
    // hide unusable menu options by setting active = false

    tmNavBar.UpdateNavBarItem( "  back" ,     getCallFromDb , 1, !isErrorTooLow  ) ;
    tmNavBar.UpdateNavBarItem( "forward" ,    getCallFromDb , 2, !isErrorTooHigh ) ;
    tmNavBar.UpdateNavBarItem( "  conf" , tmConfMain_root , 2, true ) ;
    tmNavBar.Show() ;

  }
  else displayNewCall( ) ;


  swSerial.println(F("exiting getCallFromDb() "));
}
 */



/* void  ICACHE_FLASH_ATTR resourceMonitor() {
  // runs periodically from loop() . Interval time set in loop()

  // 1. Periodically force display of latest call to displace any history record or config data in display.
  // 2. terminate long AP session.

  swSerial.println(F("\nperforming resourceMonitor() check ")) ;

  struct inWatch_t {
    bool displayShowsLatestCall ;
    uint32_t displayShowsLatestCallSetAtMs ;
  } ;

  static struct inWatch_t inWatch ;

  static byte failureCount = 0 ;
  const byte failureCountTolerance = 5 ;  // 2.5 minutes if resourceMonitor() called every 30 S.
  uint32_t ms = millis() ;

  bool allOk = true ; // assume OK


  // suspension by AP mode
  if ( resourceMonitorSuspendedAtMs > 0 ) {
    uint32_t suspensionAgeMs = millis() - resourceMonitorSuspendedAtMs ;
    if ( suspensionAgeMs > 0 &&   suspensionAgeMs  < 300000UL ) {  // 5 minutes
      swSerial.print(F(" Suspension Age mS = ")) ;
      swSerial.println( suspensionAgeMs   ) ;
      return ; // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }
    else resourceMonitorSuspendedAtMs = 0 ; // cancel suspension
  }

  // v0.73
  if ( displayShowsLatestCall == false ) {
    if ( inWatch.displayShowsLatestCall == false ) {
      inWatch.displayShowsLatestCall = true ;
      inWatch.displayShowsLatestCallSetAtMs = ms ;
    }
  }
  else inWatch.displayShowsLatestCall = false ;

  if ( inWatch.displayShowsLatestCall && ms - inWatch.displayShowsLatestCallSetAtMs > 120000 && dbOnline  )  // 2 mins
  {
    swSerial.print(F("forcing retrieval of latest call from DB")) ;
    swSerial.print(F("1. displayShowsLatestCall= ")) ;  swSerial.println ( displayShowsLatestCall  ) ;
    // force display of latest call from db
    AFSK_suspend() ;
    getCallFromDb( 0 ) ;
    swSerial.print(F("2. displayShowsLatestCall= ")) ;  swSerial.println ( displayShowsLatestCall  ) ;
    if ( ! NavBar::navBarSessionActive ) AFSK_resume() ;
  }


  noInterrupts() ;
  uint32_t demodLastActiveAtMsCopy = demodLastActiveAtMs ; // set in AFSK
  interrupts() ;

  if (  ms - demodLastActiveAtMsCopy >  20000UL  )  {  // 20 seconds
    // A long configuration session will be terminated.
    swSerial.print(F("demodLastActiveAtMs > 20 Seconds. mS= ")) ;  swSerial.println ( ms - demodLastActiveAtMsCopy  ) ;
    allOk = false ;
    if (  ms - demodLastActiveAtMsCopy >  60000UL  ) {  // 60 seconds
      // attempt to restart demodulator. Terminates long configuration sessions.
      swSerial.println(F("attempt to restart demodulator. Terminates any configuration session.")) ;
      NavBar::navBarSessionActive = false ;
    }
  }

  if ( config.networkMode == true ) {

    if ( WiFi.status() != WL_CONNECTED ) {
      swSerial.println(F("WiFi not connected. Attempting to connect")) ;
      setupSTA( true ) ;
      wlanAvailable = false ;
      allOk = false ;
    }
    else {
      swSerial.println(F("perform keep alive test ")) ;
      WiFiClient client;
      swSerial.print(F("\nconnecting to "));  swSerial.println( config.remoteHost );

      if (client.connect( config.remoteHost , 80 )) {
        swSerial.println(F("connection OK"));
        dbOnline = true ;
      }
      else  {
        swSerial.println(F("connection failed"));
        allOk = false ;
        dbOnline = false ;
      }
    }
  }
  if ( allOk ) {
    failureCount = 0 ;  //reset
  }
  else {
    swSerial.print(F("failure registered. # ")); swSerial.println( failureCount ) ;
    failureCount ++ ;
    if ( failureCount > failureCountTolerance ) {
      swSerial.println(F("Multiple failures. Restarting using WDT"));
      while ( true ) ; // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    }
  }
}
 */

void ICACHE_FLASH_ATTR handleRingDetector( bool force ) {
  //
  // force : not used
  //
  //   if in short ring burst and in alternative 1.0 second slot, show "ringing"
  //   else if in new call, show "new call"
  //   else nothing
  //

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

  // don't need
  /* if ( ms - lastDisplayUpdateAtMs > 500 )  {
    lastDisplayUpdateAtMs += 500 ;

    if ( displayNow != lastDisplayed ) {
      lastDisplayed = displayNow ;
      tft.fillRect(  190, 150, 130, 40, ILI9341_BLACK ) ;
      tft.setTextSize( 2 ) ;

      if ( displayNow == RINGING ) {
        tft.setTextColor( ILI9341_GREENYELLOW   ) ;  //  v0.72
        tft.setCursor( 192, 164 ) ;
        tft.print(F("*RINGING*")) ;  //  v0.72 F()
        tft.setTextColor( ILI9341_WHITE   ) ;
      }
      else if  ( displayNow == NEWCALL ) {
        tft.setTextColor( ILI9341_CYAN ) ;
        tft.setCursor( 192, 164 ) ;
        tft.print(F("*new call*")) ;
        tft.setTextColor( ILI9341_WHITE   ) ;
      }
    }
  } */


  // relay handling v0.71

  // globals:
  // inShortRingBurst
  // inLongRingBurst
  // spamFlag

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
      // transition into longRingBurst
      // clear spamIndicator if set.
      spamFlag = false ; // also cleared in loop()
    }
    else {
      // transition out of longRingBurst
      //pinMode( relayPin, OUTPUT ) ;  // ??
      digitalWrite( relayPin, LOW ) ;
    }
    inLongRingBurstLast = inLongRingBurst ;
  }

  if ( inLongRingBurst && spamFlag ) {
    digitalWrite( relayPin, HIGH  ) ;
  }
}







/* =================================================================================================== */

// ==============
// setup()
// ==============



void  ICACHE_FLASH_ATTR setup() {
  // dummy - standard setup() does not appear to tolerate long activity.
}


void ICACHE_FLASH_ATTR setup1() {
  //
  // standard setup() does not appear to tolerate long activity.
  //

  swSerial.begin(115200);  // te_30
  swSerial.println( "version: " ) ;
  swSerial.println( versionNr ) ;

  SPI.begin( );

  pinMode( relayPin, OUTPUT ) ;
  digitalWrite( relayPin, LOW ) ;


  configSetup() ;
  config.runNumber += 1 ;   // count system restarts

  eepromDump() ; // resave to eeprom
  eepromFetch() ;
  PrintEepromVariables() ;

  // don't need
  /* if ( ! config.networkMode )
  {
    // attempt to prevent auto sign on to wlan using cached credentials
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }

  screen_setup() ;

  tft.setTextSize(2);
  tft.setCursor(0, 10);
  tft.println( "version: " ) ;
  tft.println( versionNr );
  tft.println(  );

  tft.println( "Screen calbration" ) ;
  tft.println( "   Please wait . ." ) ;
  delay(2000) ;
  tft.fillScreen(ILI9341_BLACK);

  touchScreenCalibration() ;

  // prepare standard navbar to return to config screen.
  tmReturnToConfigRoot.UpdateNavBarItem(  " " ,      NULL ,   0, false ) ;
  tmReturnToConfigRoot.UpdateNavBarItem(  " " ,      NULL ,   1, false ) ;
  tmReturnToConfigRoot.UpdateNavBarItem(  " Config" ,   tmConfMain_root ,    2, true ) ;

  delay ( 1000 ) ;  // give the user time to read the message, then proceed.

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 10);
  tft.println() ;
  tft.print( F("exiting setup()" ) ) ;

  delay ( 1000 ) ;  // give the user time to read the message, then proceed.


  if ( config.networkMode ) {
    setupSTA( false ) ;
    WebServerSetup() ;   // for AP mode if required
  }

  // currentDbCallId = 0 ;  // force get of newest call (if any)
  if ( config.networkMode ) getCallFromDb( 0 ) ; //  latest call / db format
  else {
    strcpy( call.number , "No Calls"  ) ;
    displayNewCall( ) ;
  }
 */
  
  delay( 10000 ) ;  // v1.00 10 seconds to stop continuous crash  (was 20) could possibly be optimised
  //   otherwise screen config buttons are 'dead' for this time after appearing
  
  // setup timer and ADC. From here on, SPI slaves (TS, TFT & ADC ) must be interlocked.
  AFSK_init( &modemTST ) ;

  swSerial.print( F("\nexiting setup()" )) ;

}


// ==============
// loop()
// ==============

void loop() {

  static DemodState_t state_last ;
  static bool setupRun = false ;

  if ( ! setupRun ) {
    // workaround for crash in prolonged setup()
    setup1() ;
    setupRun = true ;
  }


  // don't need
  // in debugMode 1 the demod output is raw/unparsed so we exit before the parser stage
  // touch screen locked out - requires restart to clear.
  /* static int charsPrinted = 0 ;
  if ( debugMode == 1 )
  {
    static bool runOnce = false ;
    if ( ! runOnce ) {
      runOnce = true ;
      AFSK_resume() ;
    }
    if ( modemTST.curr_bitReady == true  ) {
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
    return ;  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  } */


  yield() ;

  uint32_t ms = millis() ;

  static uint32_t lastUtlityRun = 0 ;
  if ( ! modemTST.adcSpiExclusive   &&  ms - lastUtlityRun >= 100 ) {
    lastUtlityRun = ms ;
    // option: web server not active in STA mode. Please remove condition to change behaviour.
    // if ( apMode) server.handleClient();
    // don't need
    //server.handleClient();
  }





  // ==============
  // State: READY (set in AFSK from state INIT)
  // ==============

  // use this section to minimise impact on demodulator
  // modemTST.adcSpiExclusive set if demodulator is decoding a caller ID

  if ( modemTST.demodState == dmREADY ) {

    if ( state_last != dmREADY ) {
      if ( debugMode > 1 ) {
        swSerial.println( F("\n\n == == == == == == == == ") ) ;
        swSerial.print( F("\n > dmREADY") ) ;
      }
    }


    // v0.44
    static uint32_t lastTsScanAtMs = 0 ;
    if ( ! modemTST.adcSpiExclusive   && millis() - lastTsScanAtMs > 100) {    // was 50, && NavBar::ptrCurrentActiveNavBar != NULL 
      AFSK_suspend() ;

      handleRingDetector( true ) ;  // v0.72 writes to TFT so suspend demod required.

      // test if a NavBar button has been pressed and execute entire callback chain
      //NavBar::ptrCurrentActiveNavBar->EventWatcher() ;

      // do not free demod during a user configuration session. Please remove condition if not required.
      // don't need
      //if ( ! NavBar::navBarSessionActive ) AFSK_resume() ;
      lastTsScanAtMs = ms ;
    }


    static uint32_t lastLdrAtMs = 0 ;
    const uint32_t ldrInterval = 1000UL ;  // x seconds
    if ( ms - lastLdrAtMs > ldrInterval && ! modemTST.adcSpiExclusive ) {
      // swSerial.print( F(" analogRead LDR= ") ) ;  swSerial.print( analogRead( A0 )  ) ;
      // don't need
      //setBacklightPwm () ;
      lastLdrAtMs = ms ;
    }


    static uint32_t lastStatsDumpAtMs = 0 ;
    const uint32_t statsDumpInterval = 30000UL ; // 30 seconds
    if ( ms > statsDumpInterval && ms - lastStatsDumpAtMs > statsDumpInterval ) {
      // don't need
      //resourceMonitor() ;
      lastStatsDumpAtMs =  ms ;
    }


    state_last = dmREADY ;
  }


  // ==============
  // State: ALT_MARK_SPACE (set in AFSK)
  // ==============

  else if ( modemTST.demodState == dmALT_MARK_SPACE ) {

    if ( state_last != dmALT_MARK_SPACE ) {

      if ( debugMode > 1 )  swSerial.print( F("\n > dmALT_MARK_SPACE") ) ;

      spamFlag = false ;  // also cleared in readMcp3002Ch1()
      //displayShowsLatestCall = false ;  // v0.74

    }
    state_last = dmALT_MARK_SPACE ;
  }


  // ==============
  // State: ALL_MARKS (set in AFSK)
  // ==============

  else if ( modemTST.demodState == dmALL_MARKS ) {

    if ( state_last != dmALL_MARKS ) {

      if ( debugMode > 1 )  swSerial.print( F("\n > dmALL_MARKS") ) ;

    }
    state_last = dmALL_MARKS;
  }

  // ==============
  // State: DATA (set in AFSK)
  // ==============

  else if ( modemTST.demodState == dmDATA ) {
    //
    // This section handles reading the demodulator bit queue.
    //
    // Avoid blocking code in this section. No Serial Prints except warning/errors else it may
    // not run fast enough to get all the bits from the demodulator and cause a queue overflow.
    //

    if ( state_last != dmDATA   )  {
      // clean call structure
      for ( byte i = 0  ; i < sizeof ( call.allBytes ) ; i++ ) {
        call.allBytes[i] = 0 ;
      }
      if ( debugMode > 1 ) swSerial.print( F("\n > dmDATA") ) ;
    }

    if ( ! fifo_isempty_locked( &modemTST.demodFifo ) ) {
      // something in demodulator queue - take it
      bool inBit =  fifo_pop_locked( &modemTST.demodFifo ) ;

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
    state_last =  dmDATA ;
  }

  // ==============
  // State: CLEANUP  (set here (loop()) )
  // ==============

  else if ( modemTST.demodState == dmCLEANUP ) {
    //
    // we're at the end of the data stream
    // demod can be suspended from here on without checking.

    AFSK_suspend() ;
    callsThisRun += 1 ;

    int mql = fifo_maxQuLen( &modemTST.demodFifo ) ;
    swSerial.print(F("max demod Q len: ")) ;
    swSerial.print( mql ) ;

    swSerial.print( F("\nCHECKSUM  0x" )) ;
    swSerial.print( call.checksumCalc  , HEX ) ;
    if  ( call.checksumCalc  ) {
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


    // v0.51
    // don't need
    /* if ( config.networkMode == true ) {
      postCall()  ;
      // currentDbCallId = 0 ; // set anyway in postCall() ?
      if ( dbOnline ) {

        getCallFromDb( 0 ) ; //  latest call / db format

      }
      else {
        // failed to post call.
        displayNewCall() ; // offline format
      }
    }
    else {
      displayNewCall() ; // offline format
    } */

    // handleRingDetector( true ) ;  // v0.71

    cli() ;
    modemTST.demodState = dmINIT ;
    sei() ;
    state_last =  dmCLEANUP ;
    AFSK_resume() ;
  }

  // ==============
  // State: ERRORx (set in AFSK and here (loop()) )
  // ==============

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

      swSerial.print(  F("Bad Stopbits= " ) ) ;
      swSerial.println( call.CntBadStopBits ) ;

      swSerial.print( F("Parser Errors= " ) ) ;
      swSerial.println( call.CntParserErrors ) ;

      swSerial.print( F("Checksum = " )) ;
      swSerial.print( call.checkSumStatus ) ;

    }
    //displayNewCallError() ;

    cli();
    modemTST.demodState = dmINIT ;
    sei();
    state_last = dmERRORx ;
    AFSK_resume() ;
  }
}



