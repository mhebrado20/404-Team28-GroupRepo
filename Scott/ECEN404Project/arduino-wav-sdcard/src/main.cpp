#include <Arduino.h>
#include <stdio.h>
// #include <pgmspace.h>
// #include <FreeRTOS.h>
#include <I2SMEMSSampler.h>
#include <ADCSampler.h>
#include <I2SOutput.h>
#include <DACOutput.h>
// #include <SDCard.h>
// w#include "SPIFFS.h"
#include <WAVFileReader.h>
#include <WAVFileWriter.h>
#include "config.h"
#include <Streaming.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define ONOFFHOOK_PIN 14
//#define ONOFFHOOK_WHYS_PIN 27

void printHex(unsigned char* data, int len) 
{
  for (int i = 0; i < len; i++, data++) {
    Serial.print("0x");
    if ((unsigned char)*data <= 0xF) Serial.print("0");
    Serial.print((unsigned char)*data, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void serialize(const char *fname)
{
  File fp = SD.open(fname, FILE_READ);

  unsigned char data[1024];

  while (fp) {
    for (int i = 0; i < 1023; i++) {
      if (fp.read() == '\n') {
        fp.close();
        break;
      }
      data[i] = fp.read();
    }

    printHex(data, 1024);
  }
}

void wait_for_offhook()
{
  //Serial.println((digitalRead(ONOFFHOOK_PIN)));
  while ((digitalRead(ONOFFHOOK_PIN) == HIGH))
  {
    //vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void record(I2SSampler *input, const char *fname)
{
  int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
  printf("Start recording");
  input->start();
  // open the file on the sdcard
  File fp = SD.open(fname, FILE_WRITE);
  // create a new wave file writer
  // WAVFileWriter *writer = new WAVFileWriter(fp, input->sample_rate()); //need to replac
  // write out the header - we'll fill in some of the blanks later
  
  // keep writing until the user releases the button
  while ((digitalRead(ONOFFHOOK_PIN) == LOW))
  {
    int samples_read = input->read(samples, 1024);
    int64_t start = esp_timer_get_time();
    // writer->write(samples, samples_read);
    // fwrite(samples, sizeof(int16_t), samples_read, fp);
    fp.write((uint8_t *)samples, sizeof(int16_t) * samples_read);
    int64_t end = esp_timer_get_time();
    printf("Wrote %d samples in %lld microseconds", samples_read, end - start);
    printf("\n");
  }
  // stop the input
  // printf("\n");
  // printf("exited loop\n");
  input->stop();
  // and finish the writing
  // writer->finish(); //need to replace
  // now fill in the header with the correct information and write it again
  // printf("before close\n");
  fp.close();
  // printf("after close\n");
  // delete writer;
  // printf("before free sample\n");
  free(samples);
  // printf("Finished recording\n");
  // printf("Finished recording");
}

/*
void play(Output *output, const char *fname)
{
  int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
  // open the file on the sdcard
  FILE *fp = fopen(fname, "rb");
  // create a new wave file writer
  WAVFileReader *reader = new WAVFileReader(fp);
  printf("Start playing");
  output->start(reader->sample_rate());
  printf("Opened wav file");
  // read until theres no more samples
  while (true)
  {
    int samples_read = reader->read(samples, 1024);
    if (samples_read == 0)
    {
      break;
    }
    // ESP_LOGI("Read %d samples", samples_read);
    output->write(samples, samples_read);
    // ESP_LOGI("Played samples");
  }
  // stop the input
  output->stop();
  fclose(fp);
  delete reader;
  free(samples);
  printf("Finished playing");
}
*/

void main_task(void *param)
{
  printf("Starting up");

  // USE_SPIFFS
  // printf(TAG, "Mounting SPIFFS on /sdcard");
  // SPIFFS.begin(true, "/sdcard");

  printf("Mounting SDCard on /sdcard");
  // new SDCard("/sdcard", PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS);
  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  // ESP_LOGI("Creating microphone");
  I2SSampler *input = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_4, i2s_adc_config);


/*
#ifdef USE_I2S_SPEAKER_OUTPUT
  Output *output = new I2SOutput(I2S_NUM_0, i2s_speaker_pins);
#else
  Output *output = new DACOutput(I2S_NUM_0);
#endif
*/

  //gpio_set_direction(GPIO_BUTTON, GPIO_MODE_INPUT);
  //gpio_set_pull_mode(GPIO_BUTTON, GPIO_PULLDOWN_ONLY);

  while (true)
  {
    //Serial.println((digitalRead(ONOFFHOOK_PIN)));
    // wait for the user to push and hold the button
    wait_for_offhook();
    /*
    if (GPIO_NUM_14 != 1) {
      record(input, "/test.raw");
    }
    */
    record(input, "/test.raw");
    // wait for the user to push the button again
    /*
    wait_for_button_push();
    play(output, "/sdcard/test.wav");
    */
    vTaskDelay(pdMS_TO_TICKS(1000));
    serialize("/test.raw");
  }
}

void setup()
{
  Serial.begin(115200);
  
  pinMode(ONOFFHOOK_PIN, INPUT);
  //pinMode(ONOFFHOOK_WHYS_PIN, INPUT);
  
  xTaskCreate(main_task, "Main", 4096, NULL, 0, NULL);
}

void loop()
{
}