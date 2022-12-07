## Documentation on ESP32 state machine, written in C++. This README is the most up to date version of what each function does ##

* AFSK.cpp / AFSK.h *
AFSK.cpp and AFSK.h are adapted code files developed by Mark Qvist and 6v6gt. Since 6v6gt wrote these files using an ESP-8266 and the ESP-8266 does not have an internal ADC pin, he had to use an external ADC module. I tried to implement Evan Krall’s way of taking ADC samples through the ESP-32’s I2S (section 4.1.1).

The ADC ISR drives the AFSK demodulator; it decodes the input signal into 0s and 1s, strips the start and stop bits, drops bit stuffing, and handles the endian conversion. It recognizes the channel seizure and mark block, which comes before the data stream, and then strips these out. The remaining data stream is then passed to the byte assembler.

# AFSK_hw_init #
AFSK_hw_init is a function written by Mark Qvist to interface I2S and the ESP32’s ADC channel.


* Config.cpp / Config.h *
Config.cpp and Config.h was adapted from 6v6gt’s code from his “Arduino Esp8266 Based Telephone Caller ID System With Anti-Spam Function” project.

Config.h also has a configuration setting to record a call on an SD Card.


* constants.h *
This file is used to store constants of the ESP32 (adapted from Mark Qvist).


* device.h *
This file was written to configure the ESP32 (adapted from Mark Qvist) for interfacing I2S with the ADC channel.


* FIFO.h *
6v6gt wrote FIFO to organize the incoming bit data by First In, First Out.


* Globals.h *
Globals file was written to assist mainly with debug modes for this project (adapted from 6v6gt).


* main.cpp *
The main file is where the phone and caller identification state machine are written.

# printHex #
printHex takes an array and the length of that array as inputs and then prints out a converted hex value to the serial port

# serialize #
Serialize takes a file name as a text input, opens that file, and reads the data in the file to the serial port as hex data. The .eof() function was not working properly with the SD card library during testing, so the function waits to read a ‘/n’ character to signify that it has reached the end of the file and should stop printing to the serial port.

# wait_for_offhook #
wait_for_offhook turns the hardware subsystems off hook voltage signal into a button to start and stop sampling through the ADC

# record #
Record allocates memory for the samples array, then starts the ADCSampler task. Following this it opens a file on the SD card and then when the signal for ONOFFHOOK_PIN goes HIGH it begins sampling the ADC and writing the samples to a file in the .raw audio format. When the ONOFFHOOK_PIN goes LOW the function stops recording, closes the file on the SD card, and deallocates the memory on the ESP32 for the array.

# cidSM #
The caller ID state machine is code adapted from 6v6gt. This state machine includes the
following functions:

1. void AFSKReady(DemodState_t lastAFSKState, uint32_t mstime)
2. void AFSKAltMark(DemodState_t lastAFSKState)
3. void AFSKAllMarks(DemodState_t lastAFSKState)
4. void AFSKData(DemodState_t lastAFSKState)
5. void AFSKCleanUp(DemodState_t lastAFSKState)
6. void AFSKErrorx(DemodState_t lastAFSKState)

Caller ID is acquired from the AFSKCleanUp function. This data is sent to the database and,
at the same time, displayed on the 128x32 OLED display.

# onHookNoCall #
The first phone state is when the phone is on-hook and is waiting for a call. It detects when the RING pin is triggered (which means the phone is ringing) and then suppresses the first ring for downstream phones. While the ring is suppressed, cidSM() is called to decode caller ID. If the caller ID matches with a scammer, the ESP-32 will receive a JSON packet from the database to play a fax tone or do an NCID hang-up. The LEDs will also indicate red when this happens. If the caller ID matches with a known or unknown person, the LEDs will indicate green, and The NCID Defender device will let the call keep ringing until the user picks up. However, if the ringing times out after some time and the phone is not picked up, the phone is automatically hung up, and the next state is onhook_callend.

If the user or the ESP-32 receives a JSON packet from making an outgoing call from NCID, the next state is offhook_outgoing.

# offHookCallConn #
The second phone state is when the call is connected. Immediately, a JSON packet is sent to NCID to indicate the call start, and recording of the call begins. If the user hangs up, the next state is onhook_callend. If a howler tone is detected, the ESP-32 contacts NCID to alert the user; when on-hook is detected, the next state is onhook_callend.

While the first call is ongoing, this function should look out for any DTMF key tones and incoming calls. These have not yet been implemented at the time of writing this report.

# offHookOutgoing #
The third phone state is when the user is making an outgoing call. As soon as the phone goes off-hook, NCID will receive a JSON packet indicating the start of the call, and recording will begin.

This function should also be looking for a solid or stutter tone. If DTMF pressed was a valid phone number, the next state is offhook_callconn; if not, the next state is onhook_callend. These have not yet been implemented at the time of writing this report.

# onHookCallEnd #
The last phone state is when the call has ended, and the user puts the phone back on-hook. The recording is ended and sent to the database. A JSON packet is also sent to NCID with an indication of the end time of the call. The LEDs are reset to no color, and the next state is onhook_nocall.

# setup #
The setup function sets up input and output pins, the SD card, the OLED display, and WS2818b LEDs. Then, the AFSK and DTMF libraries so that their functions are ready to be called at any time. Lastly, the phone and AFSK states are set to their initial states.

# loop #
The loop function holds the phone state machine. The four states are onhook_nocall, offhook_callconn, offhook_outgoing, and onhook_callend.


* PhoneDTMF.cpp / PhoneDTMF.h *
This library was adapted from Adrianotiger to detect DTMF key pad tones easily. This library has not been used at the time of writing this report.


* XMDF.cpp / XMDF.h *
XMDF files were adapted from 6v6gt to parse incoming bits. parseDemodBitQueue(bool inbit) collects bits from the demod queue and assembles the bytes. The bytes are then handed over to the byte parser parseMdmf(). This function returns either a 0 (no state change), 1 (task completed), or 2 (error). ParseMdmf(byte index, byte value) is called with every byte found in the data stream, including xDMF header byte and except the checksum byte.