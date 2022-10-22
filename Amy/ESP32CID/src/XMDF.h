#ifndef XMDF_H
#define XMDF_H

#include "Arduino.h"
#include "Globals.h"


const byte LENGTH_CALL_RECORD_DATE = 8 ;
const byte LENGTH_CALL_RECORD_NUMBER = 20 ;
const byte LENGTH_CALL_RECORD_NAME = 25 ;


typedef union {
  struct  {
    char date[ LENGTH_CALL_RECORD_DATE + 1 ] ;        // allow 1 for end marker
    char number[ LENGTH_CALL_RECORD_NUMBER + 1 ] ;
    char name[ LENGTH_CALL_RECORD_NAME + 1 ] ;
    byte checkSumStatus ;
    byte CntBadStartBits ;
    byte CntBadStopBits;
    byte CntParserErrors;
    uint8_t byteWork ;                                // current byte being assembled
    uint8_t dataByteCount ;                           // count of data bytes found so far
    uint8_t byteWorkBitIndex ;                        // index into current byte  0=start; 1-8 bits ; 9 = stop. Only 1-8 are stored.
    uint16_t dataPacketLength = 0 ;
    uint16_t checksumCalc ;                           // total of all values including checksum itself
  } ;
  byte allBytes [ 70 ] ;  // !!update this if changing struct. Must be >= sizeof( struct )
} Call_t ;


extern Call_t call ;

typedef enum  {
  MDMF = 1,
  SDMF,
  OTHER ,
} RecordFormat_t ;

extern RecordFormat_t recordFormat ;



// prototypes

bool parseMdmf( byte index , byte value ) ;
byte parseDemodBitQueue( bool inBit ) ;


#endif
