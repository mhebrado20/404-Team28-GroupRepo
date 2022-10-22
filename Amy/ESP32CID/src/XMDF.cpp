#include "XMDF.h"

RecordFormat_t recordFormat;
Call_t call;

byte parseDemodBitQueue(bool inBit) {
  //
  // Collects bits from demod queue and assembles bytes.
  // The bytes are handed over to the byte parser parseMdmf()
  //
  // return codes:
  //    0: no state change - remains Data
  //    1: task completed - change state to CLEANUP
  //    2: error - change state to Error
  //
  // Avoid blocking code in this section. No Serial Prints except warning/errors else it may
  // not run fast enough to get all the bits from the demodulator.


  // if (debugMode > 3)  swSerial.print(inBit);  // printing every bit is too heavy ?  // te_57

  byte rc = 0;

  if (call.byteWorkBitIndex == 0) { // start bit
    if (inBit) {  // 0 start bit expected
      if (call.CntBadStartBits < 255) {
        call.CntBadStartBits++;
      }
      
      if (debugMode > 1) {
        Serial.println(F("\nbad start bit"));
      }
    }
    
    call.byteWorkBitIndex = 1;
  } else if (call.byteWorkBitIndex == 9) { // stop bit
    if (! inBit) {  // 1 stop bit expected
      if (call.CntBadStopBits < 255) {
        call.CntBadStopBits++;
      }
      if (debugMode > 1) {
        Serial.println(F("\nbad stop bit"));
      }
    }

    //
    //  We are at the end of a byte so process it.
    //

    if (debugMode > 2)
    {
      Serial.println();
      Serial.print(F("["));
      Serial.print(call.dataByteCount);
      Serial.print(F("]"));
      if (call.byteWork >=  0x20  && call.byteWork <= 0x7E) {   //printable ??
        Serial.print((char) call.byteWork);
      } else {
        Serial.print(" ? ");
      }
      
      Serial.print(F(" ; (hex) = 0x"));
      Serial.print(call.byteWork , HEX);
    }

    call.checksumCalc += call.byteWork;

    bool pRc = parseMdmf(call.dataByteCount, call.byteWork); // must be called for every byte.
    
    if (pRc && call.CntParserErrors < 255) {
      call.CntParserErrors ++;
    }

    if (call.dataByteCount == 0) {
      // at packet header
      if (debugMode > 1) {
        Serial.print(F("\npkt hdr"));
      }

      if (call.byteWork == 0x80) {
        recordFormat = MDMF;
      } else  {
        rc = 2;  // error (pkt header not found)
        if (debugMode > 1)
        {
          Serial.print(F("\nbad pkt hdr: "));
          Serial.print(call.byteWork , BIN);
        }
      }

      call.checksumCalc = call.byteWork;   // byte 0 : reset checksumcalc
    } else if (call.dataByteCount == 1) {
      call.dataPacketLength = call.byteWork;
      
      if (debugMode > 1)
      {
        Serial.print(F("\npkt size: "));
        Serial.print(call.dataPacketLength);
      }
    } else if (call.dataByteCount == call.dataPacketLength + 2) {
      // last packet
      call.checksumCalc &= 0xFF;
      call.checksumCalc ? call.checkSumStatus = 0 : call.checkSumStatus = 1;

      rc = 1;  // end - dmCLEANUP
      if (debugMode > 1) {
        swSerial.println(F("\n end of packet found"));
      }
    }

    call.dataByteCount ++;
    call.byteWorkBitIndex = 0;
  } else {
    // data received little_endian. Fill byte from high side.
    call.byteWork >>= 1;
    call.byteWorkBitIndex ++;
    if (inBit) {
      call.byteWork |= 0b10000000;
    }
  }

  return rc;
}


bool parseMdmf(byte index , byte value) {

  // must be called with every byte found in data stream including the xDMF header byte except the checksum byte
  // currently supports only MDMF with date, number and name parameter types only-.
  // Loads structure: call.record
  // Add no blocking code. Serial print only in the case of warnings or errors (especially if using softwareSerial or otherwise slow serial)


  enum MdmfDataType {
    mDATE = 0x01 ,
    mCALLER_NUMBER = 0x02 ,
    mCALLER_NAME = 0x07 ,
    mOTHER = 0xDF ,   // free according to http://www.etsi.org/deliver/etsi_en/300600_300699/30065903/01.03.01_40/en_30065903v010301o.pdf
  } mdmfDataType;


  enum ParserState {
    INIT = 1 ,
    PARAMETER_TYPE_EXPECTED = 2 ,
    PARAMETER_LENGTH_EXPECTED = 3 ,
    DATA_BYTE_EXPECTED = 4 ,
    AT_END = 5 ,
    UNRECOVERABLE_ERROR = 6,
  };


  static ParserState state;

  static unsigned int packetLength;
  static byte currentDataType;
  static byte currentDataTypeLength;
  static byte currentDataTypeEnd;
  static byte currentIndexOffset;
  static char* buff;

  bool rv = true;  // assume success

  // te_57
  if (debugMode > 2)
  {
    // swSerial.print(F("\nparser state = "));
    // swSerial.print(state);
  }


  if (index == 0) {
    state = INIT;
    // byte 0 is packet header (eg MDMF)
    // clean out buffers
    // for (byte i = 0; i < sizeof (call.allBytes); i++) {
    //  call.allBytes[i] = 0;
    // }
  } else if (index == 1) {
    packetLength = value;
    state = PARAMETER_TYPE_EXPECTED;
  } else if (index >= packetLength + 2) {
    // checksum
    state = AT_END;
  } else if (state == UNRECOVERABLE_ERROR) {
    rv = false;
  } else if (state == PARAMETER_TYPE_EXPECTED) {
    currentDataType = value;

    if (currentDataType == mDATE) {
      buff = (char*)&call.date;
      state = PARAMETER_LENGTH_EXPECTED;
    } else if (currentDataType == mCALLER_NUMBER) {
      buff = (char*)&call.number;
      state = PARAMETER_LENGTH_EXPECTED;
    } else if (currentDataType == mCALLER_NAME) {
      buff = (char*)&call.name;
      state = PARAMETER_LENGTH_EXPECTED;
    } else {
      state = UNRECOVERABLE_ERROR;
      if (debugMode > 1) {
        Serial.println(F("\nbad param type - Quit"));
      }
    }
  } else if (state == PARAMETER_LENGTH_EXPECTED) {
    currentDataTypeLength = value;
    currentDataTypeEnd = index + currentDataTypeLength;
    currentIndexOffset = index + 1;

    // validate lengths to avoid overflow
    if (currentDataType == mDATE && currentDataTypeLength > LENGTH_CALL_RECORD_DATE
        || currentDataType == mCALLER_NUMBER && currentDataTypeLength > LENGTH_CALL_RECORD_NUMBER
        || currentDataType == mCALLER_NAME && currentDataTypeLength > LENGTH_CALL_RECORD_NAME
       ) {
      state = UNRECOVERABLE_ERROR;
      if (debugMode > 1) {
        Serial.println(F("\nbad param length - Quit"));
      }
    } else {
      state = DATA_BYTE_EXPECTED;
    }
  } else if (state == DATA_BYTE_EXPECTED) {
    buff[index - currentIndexOffset] = (char) value;

    if (index == currentDataTypeEnd) {
      buff[index - currentIndexOffset + 1] = 0;   //add /0 marker
      state = PARAMETER_TYPE_EXPECTED;
    }
  }
  
  return (rv);
}




