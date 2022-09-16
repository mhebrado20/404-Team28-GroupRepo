import json
import serial
import wave

connection = serial.Serial(port="COM3", baudrate=115200)
connection.reset_input_buffer()

# open the file, name it and set it to write
audiofile = wave.open('C:/Users/sky20/Desktop/serialrecording/sound.wav', 'wb')
audiofile.setframerate(16000)  # sample rate
audiofile.setnchannels(1)  # mono(1) or stereo(2)
audiofile.setsampwidth(2)

while connection.isOpen():
    data = connection.readline()  # read the command line data
    audiofile.writeframes(data)

audiofile.close()
