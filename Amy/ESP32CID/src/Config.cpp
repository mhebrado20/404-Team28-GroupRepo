#include "Config.h"

config_t config ;


void ICACHE_FLASH_ATTR eepromFetch() {
  for ( int i = 0 ; i < EEPROMSIZE ; i++ ) {
    config.store[i] =  EEPROM.read(i);  // one byte at a time
  }
}


void ICACHE_FLASH_ATTR eepromDump() {
  for ( int i = 0 ; i < EEPROMSIZE ; i++ ) {
    EEPROM.put(i, config.store[i] );  // one byte at a time
  }
  EEPROM.commit();
}


void ICACHE_FLASH_ATTR PrintEepromVariables() {

  Serial.println (F("============================")) ;
  Serial.println (F("EEPROM DUMP:")) ;

  Serial.println ( config.eepromLayourVersion ) ;
  Serial.println ( config.runNumber ) ;
  Serial.println ( config.networkMode ) ;

  Serial.println ( config.ssid ) ;
  Serial.println ( config.psk ) ;
  Serial.println ( config.remoteHost ) ;
  Serial.println ( config.xScale ) ;
  Serial.println ( config.xShift ) ;
  Serial.println ( config.yScale ) ;
  Serial.println ( config.yShift ) ;

  Serial.println ( config.ruleIntlFrom ) ;
  Serial.println ( config.ruleIntlTo ) ;
  Serial.println ( config.ruleLocalFrom ) ;
  Serial.println ( config.ruleLocalTo ) ;

  Serial.println ( config.phpRetrievePath ) ;
  Serial.println ( config.phpStorePath ) ;

  Serial.println ( config.spamLevel ) ;

  Serial.println (F("============================")) ;

}


void ICACHE_FLASH_ATTR validateInitialiseEeprom( boolean force ) {
  /*
     If Eprom unconfigured / old version or a force is requested, the Eeprom is reset to 'factory' condition.
     Eeprom has version number.
  */
  Serial.println ( "Validating Eeprom." ) ;
  bool valid = false ;

  if ( strcmp( config.eepromLayourVersion , magicString ) == 0 ) valid = true ;
  else {
    Serial.print( "eepromCode layout version mismatch. Cleaning Eeprom. Found: " ) ;
    Serial.print ( config.eepromLayourVersion ) ;
    Serial.println() ;
  }
  if ( force ) {
    Serial.println ( "Eeprom reset requested. Cleaning Eeprom." ) ;
  }
  if ( ! valid || force ) {
    Serial.println ( "Restoring Eeprom to factory condition." ) ;
    for ( int i = 0 ; i < EEPROMSIZE ; i++ ) {
      EEPROM.put(i, 0 );   // null
    }

    // 'factory' settings. Overridden by configuration.
    //  Adapt these as required - makes testing easier if these are set
    //
    strcpy( config.eepromLayourVersion , magicString ) ;
    config.runNumber = 0 ;
    config.networkMode = false;

    // screen model specific - depepdent on rotation / flipping of touch screen
    config.xScale =  1.0 / 11.0  ;
    config.xShift =  170  ;
    config.yScale =  1.0 / 14.9 ;
    config.yShift =  220 ;

    //wlan
    strcpy( config.psk, "xxxxxxxx" ) ;  // 
    strcpy( config.ssid, "xxxxxxxx" ) ;  //
    strcpy( config.remoteHost, "www.xxxxxx.ch" ) ;  //


    strcpy( config.ruleIntlFrom , "00" ) ;
    strcpy( config.ruleIntlTo , "+" ) ;

    strcpy( config.ruleLocalFrom , "0" ) ;
    strcpy( config.ruleLocalTo , "+41" ) ;

    strcpy( config.phpRetrievePath , "/xxxxxx/getCallJson.php" ) ;
    strcpy( config.phpStorePath    , "/xxxxxx/storeCallData.php" ) ;

    config.spamLevel = '0' ;

    eepromDump() ;
    eepromFetch() ;
    PrintEepromVariables();
  }
}


void ICACHE_FLASH_ATTR configSetup() {
  EEPROM.begin(EEPROMSIZE);
  eepromFetch() ;  // get persistent variables from Eeprom
  PrintEepromVariables() ;
  validateInitialiseEeprom( false ) ;

}

// i2s config for using the internal ADC
i2s_config_t i2s_adc_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

/*
// i2s config for reading from I2S
i2s_config_t i2s_mic_Config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s microphone pins
i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

// i2s speaker pins
i2s_pin_config_t i2s_speaker_pins = {
    .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
    .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_SPEAKER_SERIAL_DATA,
    .data_in_num = I2S_PIN_NO_CHANGE};
*/