#ifndef _Config_H
#define _Config_H

#include "Arduino.h"
#include <EEPROM.h>

const int EEPROMSIZE = 256 ;

// change this if you change the format of config_t (which is stored in flash).
// If not found, the configuration reverts to "factory settings".
const char magicString[] = "te15" ;   // 4 chars   v0.72


typedef union {
  struct {
    char eepromLayourVersion[5];  // magicString
    unsigned int runNumber ; // incremented on reboot
    bool networkMode ; // false = standalone.

    //screen calibration
    float xScale ;
    float xShift ;
    float yScale ;
    float yShift ;
    //wlan
    char ssid[32];
    char psk[64];
    char remoteHost[32] ;
    // number translation rules
    char ruleIntlFrom[6] ;  // eg "00"
    char ruleIntlTo[6] ;    // eg "+"
    char ruleLocalFrom[6] ; // eg "0"
    char ruleLocalTo[6] ;   // eg "+41"
    char phpRetrievePath[32] ;
    char phpStorePath[32] ;
    char spamLevel ;

  } ;
  byte store[EEPROMSIZE] ;
} config_t ;

extern config_t config ;


// prototypes
void eepromFetch() ;
void eepromDump() ;
void PrintEepromVariables() ;
void validateInitialiseEeprom( boolean force ) ;
void configSetup() ;

#endif

