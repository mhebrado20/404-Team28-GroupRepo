#include <Arduino.h>
#include <ArduinoJson.h>
//#include <WiFi.h>
//#include <HTTPClient.h>
//#include "WiFiCredentials.h"
//#include "SPIFFS.h"
#include "I2SMEMSSampler.h"
#include "ADCSampler.h"
#include <iostream>
#include <cstdio>

//WiFiClient *wifiClientADC = NULL;
//HTTPClient *httpClientADC = NULL;
//WiFiClient *wifiClientI2S = NULL;
//HTTPClient *httpClientI2S = NULL;
ADCSampler *adcSampler = NULL;
I2SSampler *i2sSampler = NULL;

//DynamicJsonDocument recording(2048);

// replace this with your machines IP Address
//#define ADC_SERVER_URL "file:///C:/Users/sky20/Desktop/serialrecording"
//#define I2S_SERVER_URL "http://192.168.0.100:5003/i2s_samples"

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024, //changed from 1024
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s config for reading from left channel of I2S
/*
i2s_config_t i2sMemsConfigLeftChannel = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};
*/

// i2s pins
i2s_pin_config_t i2sPins = { //this type configures the pins of the microcontroller board that you have chosen to use to convert from analog to digital signals
    .bck_io_num = GPIO_NUM_32,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_35}; //change this pin to match the output pin of whatever microcontroller board you have chosen to use

// how many samples to read at once
const int SAMPLE_SIZE = 16384;

// send data to a serial port
void sendData(uint8_t *bytes, size_t count)
{
  // send them off to the serial port
  //digitalWrite(2, HIGH);
  //httpClient->begin(url);
  //httpClient->addHeader("content-type", "application/octet-stream");
  //httpClient->POST(bytes, count);
  //httpClient->end();
  //digitalWrite(2, LOW);
  DynamicJsonDocument recording(2048);
  recording["audioData"] = ((char*) bytes);
  serializeJson(recording, Serial);
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("");
}

// Task to write samples from ADC to our server
void adcWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param; //changed I2SSampler to ADCSampler
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples)
  {
    Serial.println("Failed to allocate memory for samples");
    return;
  }

  while (true)
  {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    //sendData(wifiClientADC, httpClientADC, ADC_SERVER_URL, (uint8_t *)samples, samples_read * sizeof(uint16_t));
    sendData((uint8_t *)samples, samples_read * sizeof(uint16_t));
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Started up");

  //recording["audioData"] = "";

  // launch WiFi
  /*
  Serial.printf("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("Started up");
  // indicator LED
  pinMode(2, OUTPUT);
  // setup the HTTP Client
  wifiClientADC = new WiFiClient();
  httpClientADC = new HTTPClient();

  //wifiClientI2S = new WiFiClient();
  //httpClientI2S = new HTTPClient();
  */
  // input from analog microphones such as the MAX9814 or MAX4466
  // internal analog to digital converter sampling using i2s
  // create our samplers

  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, adcI2SConfig);

  // set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);
  adcSampler->start();

}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}