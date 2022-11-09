#include "Globals.h"
#include "AFSK.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include <string.h>

extern unsigned long custom_preamble;
extern unsigned long custom_tail;
//extern int LibAPRS_vref;
//extern bool LibAPRS_open_squelch;

bool hw_afsk_dac_isr = false;
bool hw_5v_ref = false;
Afsk *AFSK_modem;

volatile uint32_t demodLastActiveAtMs = 0;
//volatile uint16_t adcValueCh1 = 0;

//volatile bool inShortRingBurst = false;
//volatile bool inLongRingBurst = false;

static xQueueHandle gpio_evt_queue = NULL;

uint16_t audio_buf1[TNC_I2S_BUFLEN];
uint16_t audio_buf2[TNC_I2S_BUFLEN];

#define FULL_BUF_LEN (DESIRED_SAMPLE_RATE * 2)
int8_t audio_buf_full[FULL_BUF_LEN];
size_t audio_buf_full_idx = 0;

bool demodRun = true; // v0.65 used to suspend demodulator code. Timer still runs.
volatile bool demodActive = false;  // interlock to ensure demod run complete

//void getADCSample();

// Forward declerations
int afsk_getchar(void);
void afsk_putchar(char c);
void receive_audio_task(void *arg);

/* void onAFSKTimer();
hw_timer_t *afskTimer = NULL;
portMUX_TYPE afskTimerMux = portMUX_INITIALIZER_UNLOCKED;
static unsigned int AFSKTimeCounter = 1;

void IRAM_ATTR onAFSKTimer(){ 
  portENTER_CRITICAL_ISR(&afskTimerMux);

  Serial.print("onCallerTimer1 ");
  Serial.print(AFSKTimeCounter);
  Serial.print(" at ");
  Serial.print(millis());
  Serial.println(" ms");

  AFSKTimeCounter++;
  portEXIT_CRITICAL_ISR(&afskTimerMux);
}

void startTimer() {
  Serial.println("Start AFSK timer.");
  // timer 0, CP = 1 ns, count up
  afskTimer = timerBegin(0, 80, true);
  // edge triggered
  timerAttachInterrupt(afskTimer, &onAFSKTimer, true);
  // 1 s
  timerAlarmWrite(afskTimer, 1000000, true);
  timerAlarmEnable(afskTimer);
} */


/* void ICACHE_RAM_ATTR getAdcSample(Afsk *afsk)
{
  // called by timer1 (9600Hz) set up in AFSK_resume() and supended in AFSK_suspend()
  // MCP2003 code loosely based on
  // https://rheingoldheavy.com/mcp3008-tutorial-02-sampling-dc-voltage/
  // but modified for the mcp3002 and to return an 8 significant bit number shifted to range -128 to 127.

  // routine supended when SPI devices (TS and TFT) which are synchronous with the main loop() are active,
  // tp prevent contention.

  // v 0.65 - we let the timer run but don't act on it. Minimise timer suspend/resume overhead.
  if (!demodRun) {
    return;
  }

  demodActive = true;  // interlock

  // don't need since we have internal ADC
  const byte MCP3002Ch0 = 0b01101000;  // start bit + openended conversion + chan 0 + MSBF (see data sheet)
  const byte MCP3002Ch1 = 0b01111000;  // start bit + openended conversion + chan 1 + MSBF (see data sheet)
  byte dataMsb0;
  byte dataLsb0;
  byte dataMsb1;
  byte dataLsb1;

  int sample8BitMidZero;
  static uint32_t runCount = 0;

  if (runCount >= 100000UL) {
    // approx 10 seconds
    demodLastActiveAtMs = millis();
    runCount = 0;
  }
  else {
    runCount++;
  }

  
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  digitalWrite(ADC_CS_PIN, LOW);
  dataMsb0 = SPI.transfer(MCP3002Ch0) & 0x03;  // 2 most significant digits in this byte
  dataLsb0 = SPI.transfer(0);                  // Push remaining data and get LSB byte return
  digitalWrite(ADC_CS_PIN, HIGH);

  digitalWrite(ADC_CS_PIN, LOW);
  dataMsb1 = SPI.transfer(MCP3002Ch1) & 0x03;  // 2 most significant digits in this byte
  dataLsb1 = SPI.transfer(0);                  // Push remaining data and get LSB byte return
  digitalWrite(ADC_CS_PIN, HIGH);

  SPI.endTransaction();

  sample8BitMidZero = (int16_t)((dataMsb0 << 6) | (dataLsb0 >> 2)) - 128;  // build 8 significant bit number in range -128 to 127
  adcValueCh1 = (int16_t)( dataMsb1 << 8 ) | dataLsb1;   // 0..1023  ring detector

  AFSK_adc_isr(AFSK_modem, sample8BitMidZero);

  demodActive = false;
} */

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/* void AFSK_hw_init(void) {
    // Configure audio input trigger pin
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = (1ULL<<GPIO_AUDIO_TRIGGER);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Configure PTT output
    gpio_set_direction(GPIO_PTT_OUT, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_PTT_OUT, 1);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    //xTaskCreate(receive_audio_task, "receive_audio_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_AUDIO_TRIGGER, gpio_isr_handler, (void*) GPIO_AUDIO_TRIGGER);

    i2s_port_t i2s_num = TNC_I2S_NUM;
    i2s_config_t i2s_config = {
       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN),
       .sample_rate =  TNC_I2S_SAMPLE_RATE,
       .bits_per_sample = TNC_I2S_SAMPLE_BITS,
       .channel_format = TNC_I2S_FORMAT,
       .communication_format = I2S_COMM_FORMAT_I2S_MSB,
       .intr_alloc_flags = 0,
       .dma_buf_count = 2,
       .dma_buf_len = 300,
       // .use_apll = 1,
       .use_apll = 0,
    };
    //install and start i2s driver
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    //init DAC pad
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    //init ADC pad
    i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);
    adc1_config_channel_atten(I2S_ADC_CHANNEL, ADC_ATTEN_DB_11);
} */


// ==============
// AFSK_init()
// ==============


void ICACHE_FLASH_ATTR AFSK_init(Afsk *afsk) {
  // Clean modem struct memory
  memset(afsk, 0, sizeof(*afsk));
  AFSK_modem = afsk;

  // Set phase increment
  afsk->phaseInc = MARK_INC;
  
  // Initialise FIFO buffers
  fifo_init(&afsk->delayFifo, (uint8_t *)afsk->delayBuf, sizeof(afsk->delayBuf));
  fifo_init(&afsk->demodFifo, (uint8_t *)afsk->demodBuf, sizeof(afsk->demodBuf));
  //fifo_init(&afsk->rxFifo, afsk->rxBuf, sizeof(afsk->rxBuf));
  //fifo_init(&afsk->txFifo, afsk->txBuf, sizeof(afsk->txBuf));

  // Fill delay FIFO with zeroes
  for (int i = 0; i < (SAMPLESPERBIT / 2); i++) {
    fifo_push(&afsk->delayFifo, 0);
  }

  afsk->demodState = dmINIT;
  // demodInitialised = true;

  //pinMode(ADC_CS_PIN, OUTPUT);

  // AFSK_resume();  // timer1 initial set to start demod.
  // v0.65
  
  //startTimer();
  //AFSK_hw_init();
}


// ==============
// AFSK_resume()
// ==============

void ICACHE_RAM_ATTR AFSK_resume() {
  /* if (debugMode == 3) {
    Serial.println(F("in AFSK_resume()"));
  } */
  // timer1: http://onetechpulse.com/esp8266-microsecond-delay-timer/
  // v0.65  only enable/disable now required ?

  demodRun = true;
}

// ==============
// AFSK_suspend ()
// ==============

void ICACHE_RAM_ATTR AFSK_suspend() {
  /* if (debugMode == 3) {
    Serial.println(F("in AFSK_suspend()"));
  } */

  demodRun = false;

  while (demodActive) {} // wait
}

/* static bool hdlcParse(Hdlc *hdlc, bool bit, FIFOBuffer *fifo) {
    // Initialise a return value. We start with the
    // assumption that all is going to end well :)
    bool ret = true;

    // Bitshift our byte of demodulated bits to
    // the left by one bit, to make room for the
    // next incoming bit
    hdlc->demodulatedBits <<= 1;
    // And then put the newest bit from the
    // demodulator into the byte.
    hdlc->demodulatedBits |= bit ? 1 : 0;

    // Now we'll look at the last 8 received bits, and
    // check if we have received a HDLC flag (01111110)
    if (hdlc->demodulatedBits == HDLC_FLAG) {
        // If we have, check that our output buffer is
        // not full.
        if (!fifo_isfull(fifo)) {
            // If it isn't, we'll push the HDLC_FLAG into
            // the buffer and indicate that we are now
            // receiving data. For bling we also turn
            // on the RX LED.
            fifo_push(fifo, HDLC_FLAG);
            hdlc->receiving = true;
            //if(!LibAPRS_open_squelch) {
            //    LED_RX_ON();
            //}
        } else {
            // If the buffer is full, we have a problem
            // and abort by setting the return value to
            // false and stopping the here.

            ret = false;
            hdlc->receiving = false;
            //LED_RX_OFF();
        }

        // Everytime we receive a HDLC_FLAG, we reset the
        // storage for our current incoming byte and bit
        // position in that byte. This effectively
        // synchronises our parsing to  the start and end
        // of the received bytes.
        hdlc->currentByte = 0;
        hdlc->bitIndex = 0;
        return ret;
    }

    // Check if we have received a RESET flag (01111111)
    // In this comparison we also detect when no transmission
    // (or silence) is taking place, and the demodulator
    // returns an endless stream of zeroes. Due to the NRZ
    // coding, the actual bits send to this function will
    // be an endless stream of ones, which this AND operation
    // will also detect.
    if ((hdlc->demodulatedBits & HDLC_RESET) == HDLC_RESET) {
        // If we have, something probably went wrong at the
        // transmitting end, and we abort the reception.
        hdlc->receiving = false;
        //LED_RX_OFF();
        return ret;
    }

    // If we have not yet seen a HDLC_FLAG indicating that
    // a transmission is actually taking place, don't bother
    // with anything.
    if (!hdlc->receiving)
        return ret;

    // First check if what we are seeing is a stuffed bit.
    // Since the different HDLC control characters like
    // HDLC_FLAG, HDLC_RESET and such could also occur in
    // a normal data stream, we employ a method known as
    // "bit stuffing". All control characters have more than
    // 5 ones in a row, so if the transmitting party detects
    // this sequence in the _data_ to be transmitted, it inserts
    // a zero to avoid the receiving party interpreting it as
    // a control character. Therefore, if we detect such a
    // "stuffed bit", we simply ignore it and wait for the
    // next bit to come in.
    //
    // We do the detection by applying an AND bit-mask to the
    // stream of demodulated bits. This mask is 00111111 (0x3f)
    // if the result of the operation is 00111110 (0x3e), we
    // have detected a stuffed bit.
    if ((hdlc->demodulatedBits & 0x3f) == 0x3e)
        return ret;

    // If we have an actual 1 bit, push this to the current byte
    // If it's a zero, we don't need to do anything, since the
    // bit is initialized to zero when we bitshifted earlier.
    if (hdlc->demodulatedBits & 0x01)
        hdlc->currentByte |= 0x80;

    // Increment the bitIndex and check if we have a complete byte
    if (++hdlc->bitIndex >= 8) {
        // If we have a HDLC control character, put a AX.25 escape
        // in the received data. We know we need to do this,
        // because at this point we must have already seen a HDLC
        // flag, meaning that this control character is the result
        // of a bitstuffed byte that is equal to said control
        // character, but is actually part of the data stream.
        // By inserting the escape character, we tell the protocol
        // layer that this is not an actual control character, but
        // data.
        if ((hdlc->currentByte == HDLC_FLAG ||
             hdlc->currentByte == HDLC_RESET ||
             hdlc->currentByte == AX25_ESC)) {
            // We also need to check that our received data buffer
            // is not full before putting more data in
            if (!fifo_isfull(fifo)) {
                fifo_push(fifo, AX25_ESC);
            } else {
                // If it is, abort and return false
                hdlc->receiving = false;
                //LED_RX_OFF();
                ret = false;
            }
        }

        // Push the actual byte to the received data FIFO,
        // if it isn't full.
        if (!fifo_isfull(fifo)) {
            fifo_push(fifo, hdlc->currentByte);
        } else {
            // If it is, well, you know by now!
            hdlc->receiving = false;
            //LED_RX_OFF();
            ret = false;
        }

        // Wipe received byte and reset bit index to 0
        hdlc->currentByte = 0;
        hdlc->bitIndex = 0;

    } else {
        // We don't have a full byte yet, bitshift the byte
        // to make room for the next bit
        hdlc->currentByte >>= 1;
    }

    //digitalWrite(13, LOW);
    return ret;
} */

// ==============
// AFSK_adc_isr()
// ==============


void ICACHE_RAM_ATTR AFSK_adc_isr(Afsk *afsk, int8_t currentSample) {
  // To determine the received frequency, and thereby
  // the bit of the sample, we multiply the sample by
  // a sample delayed by (samples per bit / 2).
  // We then lowpass-filter the samples with a
  // Chebyshev filter. The lowpass filtering serves
  // to "smooth out" the variations in the samples.

  afsk->iirX[0] = afsk->iirX[1];
  afsk->iirX[1] = ((int8_t)fifo_pop(&afsk->delayFifo) * currentSample) >> 2;

  afsk->iirY[0] = afsk->iirY[1];

  afsk->iirY[1] = afsk->iirX[0] + afsk->iirX[1] + (afsk->iirY[0] >> 1); // Chebyshev filter


  // We put the sampled bit in a delay-line:
  // First we bitshift everything 1 left
  afsk->sampledBits <<= 1;
  // And then add the sampled bit to our delay line
  afsk->sampledBits |= (afsk->iirY[1] > 0) ? 1 : 0;

  // Put the current raw sample in the delay FIFO
  fifo_push(&afsk->delayFifo, currentSample);

  // We need to check whether there is a signal transition.
  // If there is, we can recalibrate the phase of our
  // sampler to stay in sync with the transmitter. A bit of
  // explanation is required to understand how this works.
  // Since we have PHASE_MAX/PHASE_BITS = 8 samples per bit,
  // we employ a phase counter (currentPhase), that increments
  // by PHASE_BITS everytime a sample is captured. When this
  // counter reaches PHASE_MAX, it wraps around by modulus
  // PHASE_MAX. We then look at the last three samples we
  // captured and determine if the bit was a one or a zero.
  //
  // This gives us a "window" looking into the stream of
  // samples coming from the ADC. Sort of like this:
  //
  //   Past                                      Future
  //       0000000011111111000000001111111100000000
  //                   |________|
  //                       ||
  //                     Window
  //
  // Every time we detect a signal transition, we adjust
  // where this window is positioned little. How much we
  // adjust it is defined by PHASE_INC. If our current phase
  // phase counter value is less than half of PHASE_MAX (ie,
  // the window size) when a signal transition is detected,
  // add PHASE_INC to our phase counter, effectively moving
  // the window a little bit backward (to the left in the
  // illustration), inversely, if the phase counter is greater
  // than half of PHASE_MAX, we move it forward a little.
  // This way, our "window" is constantly seeking to position
  // it's center at the bit transitions. Thus, we synchronise
  // our timing to the transmitter, even if it's timing is
  // a little off compared to our own.

  // te_47 no change to currentPhase if at middle point.
  if (SIGNAL_TRANSITIONED(afsk->sampledBits)) {
    if (afsk->currentPhase < PHASE_THRESHOLD) {
      afsk->currentPhase += PHASE_INC;
    } else if (afsk->currentPhase > PHASE_THRESHOLD) {
      afsk->currentPhase -= PHASE_INC;
    }
  }

  // We increment our phase counter
  afsk->currentPhase += PHASE_BITS;

  // Check if we have reached the end of
  // our sampling window.
  if (afsk->currentPhase >= PHASE_MAX) {
    // If we have, wrap around our phase
    // counter by modulus
    //afsk->currentPhase = (afsk->currentPhase + PHASE_MAX) % PHASE_MAX;     // could be slow ? ensure not negative
    afsk->currentPhase %= PHASE_MAX;

    // Bitshift to make room for the next
    // bit in our stream of demodulated bits
    afsk->actualBits <<= 1;


    // We determine the actual bit value by reading
    // the last 3 sampled bits. If there is three or
    // more 1's, we will assume that the transmitter
    // sent us a one, otherwise we assume a zero
    uint8_t bits = afsk->sampledBits & 0x07;
    if (bits == 0x07 || // 111
        bits == 0x06 || // 110
        bits == 0x05 || // 101
        bits == 0x03    // 011
        ) {
      afsk->actualBits |= 1;
    }

    afsk->curr_bit = !(afsk->actualBits & 1);      // inverted ?
    afsk->curr_bitReady = true;    // possible test to check it is false, otherwise increment unconsumed bit count.

    if ((afsk->curr_bit == afsk->last_bit) && (afsk->curr_bit == MARKA)) {
      if (afsk->counter1111 < 255) {
        afsk->counter1111++;
      }
    } else {
      afsk->counter1111 = 0;
    }

    if ((afsk->curr_bit == afsk->last_bit) && (afsk->curr_bit == SPACEA)) {
      if (afsk->counter0000 < 0xFFFF) {
        afsk->counter0000++;
      }
    } else {
      afsk->counter0000 = 0;
    }

    if (afsk->curr_bit != afsk->last_bit) {
      if (afsk->counter0101 < 255) {
        afsk->counter0101++;
      }
    } else {
      afsk->counter0101 = 0;
    }

    if (afsk->demodState == dmINIT) {
      afsk->counter1111 = 0;
      afsk->counter0000 = 0;
      afsk->counter0101 = 0;
      fifo_flush(&afsk->demodFifo);
      afsk->demodError = 0;
      afsk->demodState = dmREADY;
    }

    // in debugMode 1, the data is presented as raw without parsing, so quit here (after setting demodState = dmREADY)  v0.64

    //if (debugMode == 1) {
    //  return;
    //}


    if (afsk->demodState == dmREADY) {
      if (afsk->counter0101 >= 20) {   // allow stabilisation
        // afsk->entered_dmALT_MARK_SPACE_atTicks = adcIsr_ticks;
        afsk->samplesIn_dmALT_MARK_SPACE = 0;
        afsk->demodState = dmALT_MARK_SPACE;
      }
    }

    if (afsk->demodState == dmALT_MARK_SPACE) {
      if (afsk->counter1111 > 10) {
        afsk->demodState = dmALL_MARKS;
      } else if (afsk->samplesIn_dmALT_MARK_SPACE > 400) {
        // we're stuck here so terminate
        afsk->demodError = 4;
        afsk->demodState = dmERRORx;
      } else {
        afsk->samplesIn_dmALT_MARK_SPACE ++;
      }
    }

    if (afsk->demodState == dmALL_MARKS) {
      if (afsk->counter1111 == 0) {
        afsk->demodState = dmDATA;
      }
    }

    if (afsk->demodState == dmDATA) {
      if (!fifo_isfull_locked(&afsk->demodFifo)) {   // locked version not required in ISR ?
        fifo_push_locked(&afsk->demodFifo, afsk->curr_bit);
      } else {
        afsk->demodError = 5;
        afsk->demodState = dmERRORx;    // demod queue full
      }
      
      if (afsk->counter0000 >= 200) {   // if we've shot wildly over the end  // te_55 was 4000
        afsk->demodError = 6;
        afsk->demodState = dmERRORx;    // state after DATA normally set in header parser (should set error code ? )
      }
    }

    afsk->last_bit =  afsk->curr_bit;

    //afsk->adcSpiExclusive =  afsk->demodState == dmALT_MARK_SPACE || afsk->demodState == dmALL_MARKS
    //                         || afsk->demodState == dmDATA  || (afsk->demodState == dmREADY && afsk->counter0101 > 2);

  }
}

extern void APRS_poll(void);
// uint8_t poll_timer = 0;
// ISR(ADC_vect) {
//     TIFR1 = _BV(ICF1);
//     AFSK_adc_isr(AFSK_modem, ((int16_t)((ADC) >> 2) - 128));
//     if (hw_afsk_dac_isr) {
//         DAC_PORT = (AFSK_dac_isr(AFSK_modem) & 0xF0) | _BV(3);
//     } else {
//         DAC_PORT = 128;
//     }

//     poll_timer++;
//     if (poll_timer > 3) {
//         poll_timer = 0;
//         APRS_poll();
//     }
// }


/* void record_audio(uint16_t *buffer) {
    size_t bytes_read;
    esp_err_t error = i2s_read(TNC_I2S_NUM, (void*) buffer, TNC_I2S_BUFLEN * sizeof(uint16_t), &bytes_read, portMAX_DELAY (3 * TNC_I2S_SAMPLE_RATE / TNC_I2S_BUFLEN / 2 / portTICK_PERIOD_MS));

    for (int i=0; i<TNC_I2S_BUFLEN; i++) {
        buffer[i] = 4095 - buffer[i];
    }

    // printf("error %d, read %d bytes\n", error, bytes_read);
} */

/* void process_audio(uint16_t *buffer) {
    // printf("processing buffer %d\n", (int)buffer);
    for (int i=0; i<TNC_I2S_BUFLEN; i += OVERSAMPLING) {
        int average = 0;
        for (int j=0; j<OVERSAMPLING && j+i < TNC_I2S_BUFLEN; j++) {
            average += buffer[i+j];
        }
        average /= OVERSAMPLING;

        average -= 717; // empirically measured hackily, we high-pass-filter later so this doesn't matter much.
        average = average * 255 / 1564;
        if (average < -128) average = -128;
        if (average > 127) average = 127;

        if (audio_buf_full_idx < FULL_BUF_LEN) {
            audio_buf_full[audio_buf_full_idx++] = average;
        }
    }
} */


/* void receive_audio_task(void *arg) {
    gpio_num_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            gpio_isr_handler_remove(GPIO_AUDIO_TRIGGER);
            uint32_t bogus;
            while (xQueueReceive(gpio_evt_queue, &bogus, 0)); // clear queue

            // grab audio lock
            // until tail N bytes of buffer are < threshold:
            //   record audio into buffer
            //   swap to other buffer
            //   send audio buffer to queue for processing
            ESP_LOG_LEVEL(ESP_LOG_INFO, "receive_audio_task", "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));

            uint16_t *buffer = audio_buf1;

            int num_recordings = 0;

            i2s_adc_enable(TNC_I2S_NUM);

            for (bool keep_recording = true; keep_recording; ) {
                num_recordings++;
                record_audio(buffer);
                process_audio(buffer);

                // printf("still recording\n");
                keep_recording = false;
                for (int i=0; i<TNC_I2S_BUFLEN; i++) {
                    if (buffer[i] > KEEP_RECORDING_THRESH) {
                        keep_recording = true;
                        break;
                    }
                }

                //switch to other buffer.
                if (buffer == audio_buf1) {
                    buffer = audio_buf2;
                    // printf("switched to audio_buf2\n");
                } else {
                    buffer = audio_buf1;
                    // printf("switched to audio_buf1\n");
                }
            }

            i2s_adc_disable(TNC_I2S_NUM);

            int running_sum = 0;
            uint8_t running_sum_len = 0;
            #define MAX_RUNNING_SUM_LEN (DESIRED_SAMPLE_RATE / 600)
            ESP_LOG_LEVEL(ESP_LOG_INFO, "receive_audio_task", "did %d recordings in %d ticks\n", num_recordings, 0);
            uint8_t poll_timer = 0;

            // viterbi(1.0);

            for (int i=0; i<audio_buf_full_idx; i++) {
                int16_t sample = audio_buf_full[i];
                running_sum += sample;
                if (running_sum_len >= MAX_RUNNING_SUM_LEN) {
                    running_sum -= audio_buf_full[i - (running_sum_len)];
                } else {
                    running_sum_len++;
                }

                sample -= running_sum / running_sum_len;
                if (sample > 127) sample = 127;
                if (sample < -128) sample = 128;

                // printf("%d ", sample);
                // if (i%20 == 19) printf("\n");

                AFSK_adc_isr(AFSK_modem, sample);
                poll_timer++;
                if (poll_timer > 3) {
                    poll_timer = 0;
                    APRS_poll();
                }
            }

            audio_buf_full_idx = 0;

            gpio_isr_handler_add(GPIO_AUDIO_TRIGGER, gpio_isr_handler, (void*) GPIO_AUDIO_TRIGGER);
        }
    }
} */