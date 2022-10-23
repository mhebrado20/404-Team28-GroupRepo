#include <Arduino.h>
#include <ArduinoJson.h>
#include "I2SMEMSSampler.h"
#include "ADCSampler.h"
#include <iostream>
#include <cstdio>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


ADCSampler *adcSampler = NULL;
I2SSampler *i2sSampler = NULL;

/*

// This function was tested as a possible alternative to uint8_t data type, converting the one byte (8-bit) unsigned integer
// value into a unsigned char array representing the same value in hex

bool to_hex(char* dest, size_t dest_len, const uint8_t* values, size_t val_len) {
    static const char hex_table[] = "0123456789ABCDEF";
    if(dest_len < (val_len*2+1)) // check that dest is large enough 
        return false;
    while(val_len--) {
        // shift down the top nibble and pick a char from the hex_table 
        *dest++ = hex_table[*values >> 4];
        // extract the bottom nibble and pick a char from the hex_table
        *dest++ = hex_table[*values++ & 0xF];
    }
    *dest = 0;
    return true;
}
*/

/*
int SineValues[256];
float ConversionFactor=(2*PI)/256;
float RadAngle;
*/

/*

// This custom class was something written as laid out by arduinoJson for overloading write functions for the serial port to display
// uncommon or unsupported data types, uint8_t is supported by arduinoJson's latest version though

struct CustomWriter {
  std::string str;

  size_t write(uint8_t c) {
    str.append(1, static_cast<char>(c));
    return 1;
  }

  size_t write(const uint8_t *s, size_t n) {
    str.append(reinterpret_cast<const char *>(s), n);
    return n;
  }
};
*/

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};


// i2s pins
i2s_pin_config_t i2sPins = { 
    .bck_io_num = GPIO_NUM_32,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_35
};


// how many samples to read at once
const int SAMPLE_SIZE = 16348; //16348


// send data to a serial port
/*********************************
 * 
 * This includes a lot of commented out debugging code and potential fixes I have tried to get this code working,
 * buf and to_hex are not solutions to the current problem but I wanted to leave them there so I know what I have tried.
 * 
 * uint8 variable is the test uint8_t array that I have been able to easily receive on the host computer side, this is received
 * by the host computer python code and converted to an array that matches the initialized values
 * 
 * all uint8 arrays must be terminated by a null character so that the array is not sent through serial as garbage data
 * intially I was testing this in the sendData function to ensure my understanding was correct but a terminating bit has been
 * added in the adcSampler->read() function
 * 
 * recording is initialized using DynamicJsonDocument which means that it assigns memory similarly to malloc as the size of the array
 * we are trying to send ciuld potentially be larger than the heap which is what StaticJsonDocument uses
 * 
 * serializeJson is the function from arduinoJson that actually sends the information through the serial port, I included a 
 * Serial.print("\n") for readability and because it enables some functionality when receiving the bits on the other side using the
 * serial port readline() command in python
 * 
**********************************/
void sendData(uint8_t *bytes, size_t count)
{
  // printf("Start CustomWriter\n");
  // CustomWriter writer;
  // printf("Create character buffer\n");
  // char *buf = (char *)malloc(count*2+1);
  // printf("Initialize JsonDoc\n");
  // DynamicJsonDocument recording(8192);
  // printf("Call to_hex function\n");
  // to_hex(buf, sizeof(buf), bytes, count);
  /*
  if(to_hex(buf, sizeof(buf), bytes, count)) {
    recording["audioData"] = buf;
    serializeJson(recording, Serial);
    Serial.print("\n");
  }
  */
  // printf("to_hex completed\n");
  // recording = recording.as<JsonVariant>();
  // printf("Print buf\n");
  uint8_t uint8[] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x00};
  // uint8[sizeof(uint8)-1] = (uint8_t)0x00;
  // uint8_t termChar = 0x00;
  DynamicJsonDocument recording(5120);
  // recording = recording.as<JsonVariant>();
  recording["audioData"] = bytes;
  // std::cout << sizeof(bytes) << std::endl;
  // std::cout << "size of array: " << (bytes) << std::endl;
  // std::cout << "size of count: " << count << std::endl;
  // recording.add((uint8_t)0x00);
  /*
  if(recording.JsonDocument::overflowed()) {
    printf("document overflowed\n");
  } else {
    printf("NOT overflowed\n");
  }
  */
  // serializeJson(recording, Serial);
  // Serial.print("\n");
  // serializeJson(terminatingChar, Serial);
  // Serial.print((String)uint8);
  // std::cout << "bytes: " << (uint8_t *)bytes << std::endl;
  // uint8_t uint8[] = {0x01, 0x02, 0x03, 0x04};
}


// Task to write samples from ADC to our server
/**********************************************
 * 
 * This function handles the writing of the data to the array, it starts by initializing a I2SSampler and an array
 * of data that is 32696 long, if the array cannot be allocated it prints an error message.
 * 
 * It then assigns the variable sampels_read with adcSampler->read, this read function will return the number of
 * samples read into the samples array. In practice this number is always the same
 * 
 * The cout statement shows that the data at this point is corrupted, I can't figure out why as every element of the
 * array is assigned within the adcSampler class function just fine according to print statements in the function
 * 
 * My thought process at this point is that it is the cast to uint8_t *  from uint16_t * or writing past the
 * boundary of the array, although I think I would have caught this by now
 * 
**********************************************/
void adcWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param; // changed I2SSampler to ADCSampler
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE + 1); //+1 to account for the terminating bit that will be added to the end
  // std::cout << "what size should be: " << (sizeof(uint16_t) * SAMPLE_SIZE) << std::endl;
  // std::cout << "what size is: " << (sizeof(*samples)) << std::endl;
  if (!samples)
  {
    Serial.println("Failed to allocate memory for samples");
    return;
  }

  while (true)
  {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    // std::cout << (sizeof(samples)) << std::endl;
    // printf("Start send data\n");
    // sendData((uint8_t *)samples, samples_read * sizeof(uint16_t));
    std::cout << "bytes: " << (uint8_t *)samples << std::endl;
  }
}


/**********************************************
 * 
 * This initializes the adcSampler, pins the adcSampler to the esp32 core and then starts the sampler
 * 
 * This is the function I am the least comfortable with changing stuff, I have the functions called exactly as atomic14 recommended
 * for adcSampler, I don't completely understand xTaskCreatePinnedToCore but its more likely I am messing up somewhere that I am assigning
 * data values like the adcWriterTask function
 * 
**********************************************/
void startWriteToSerial()
{
  // printf("adcSampler intialize\n");
  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, adcI2SConfig);
  // adcSampler->configureI2S();
  // printf("adcSampler created\n");
  // set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  // printf("Begin pinned to core task\n");
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);
  adcSampler->start();
}

/*
void dacSineConversion()
{
  for(int i=0;i<256;i++)
    dacWrite(25,SineValues[i]);
}
*/

void setup()
{
  // printf("Open serial port\n");
  Serial.begin(115200);
  // printf("Serial port opened\n");

  /*
  for(int MyAngle=0;MyAngle<256;MyAngle++) 
  {
    RadAngle=MyAngle * ConversionFactor;
    SineValues[MyAngle]=(sin(RadAngle)*127)+128;
  }
  */

  // printf("Begin write serial\n");
  startWriteToSerial();
}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
  // dacSineConversion();
}