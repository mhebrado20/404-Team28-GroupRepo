#include <Arduino.h>
#include <ArduinoJson.h>
#include "I2SMEMSSampler.h"
#include "ADCSampler.h"
#include <iostream>
#include <cstdio>


ADCSampler *adcSampler = NULL;
I2SSampler *i2sSampler = NULL;

int SineValues[256];
float ConversionFactor=(2*PI)/256;
float RadAngle;

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 9600,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s pins
i2s_pin_config_t i2sPins = { // this type configures the pins of the microcontroller board that you have chosen to use to convert from analog to digital signals
    .bck_io_num = GPIO_NUM_32,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_35};

// how many samples to read at once
const int SAMPLE_SIZE = 16384;

// send data to a serial port
void sendData(uint8_t *bytes, size_t count)
{
  DynamicJsonDocument recording(2048);
  recording["audioData"] = bytes;
  serializeJson(bytes, Serial);
  Serial.println("\n");
}

// Task to write samples from ADC to our server
void adcWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param; // changed I2SSampler to ADCSampler
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples)
  {
    Serial.println("Failed to allocate memory for samples");
    return;
  }

  while (true)
  {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    sendData((uint8_t *)samples, samples_read * sizeof(uint16_t));
  }
}

void initializeSerialPort()
{
  Serial.begin(115200);
}

void startWriteToSerial()
{
  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, adcI2SConfig);

  // set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);
  adcSampler->start();
}

void dacSineConversion()
{
  for(int i=0;i<256;i++)
    dacWrite(25,SineValues[i]);
}

void setup()
{
  initializeSerialPort();

  /*
  for(int MyAngle=0;MyAngle<256;MyAngle++) 
  {
    RadAngle=MyAngle * ConversionFactor;
    SineValues[MyAngle]=(sin(RadAngle)*127)+128;
  }
  */


  startWriteToSerial();
}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
  // dacSineConversion();
}