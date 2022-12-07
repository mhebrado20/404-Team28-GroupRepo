README

## Documentation on ESP32 code for capturing audio, written in C++. This README is the most up to date version of what each function does ##

# main.cpp #
Main.cpp contains the majority of the code that makes audio sampling through the ADC possible. It sets up the SD card, calls the ADCSampler code from the lib directory, 
and then pins the ADCSampler task to an ESP32 cpu core so that it isn’t interrupted. The code then waits until the phone goes off hook (is picked up and answered) to 
begin sampling the values from the ADC and writing them to the SD card.


# printHex #
    printHex takes an array and the length of that array as inputs and then prints out a converted hex value to the serial port


# serialize #
Serialize takes a file name as a text input, opens that file, and reads the data in the file to the serial port as hex data. The .eof() function was not working 
properly with the SD card library during testing, so the function waits to read a ‘/n’ character to signify that it has reached the end of the file and should 
stop printing to the serial port.


# wait_for_offhook #
wait _for_offhook turns the hardware subsystems off hook voltage signal into a button to start and stop sampling through the ADC


# record #
Record allocates memory for the samples array, then starts the ADCSampler task. Following this it opens a file on the SD card and then when the signal for ONOFFHOOK_PIN 
goes HIGH it begins sampling the ADC and writing the samples to a file in the .raw audio format. When the ONOFFHOOK_PIN goes LOW the function stops recording, closes the 
file on the SD card, and deallocates the memory on the ESP32 for the array.


# main_task #
The main_task function initializes the SD card, the ADCSampler, and the while loop that will ‘listen’ for the wait_for_offhook function to trigger the recording to start. 
It then calls the function serialize following the recording ending.


# setup #
Setup initializes the serial port, the ONOFFHOOK_PIN pinMode, and pins the main_task function to an ESP32 CPU core so that it will constantly run on one of the cores.
