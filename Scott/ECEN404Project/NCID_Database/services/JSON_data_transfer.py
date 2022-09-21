import json
import serial
import wave
import io
import time
import pyaudio
import numpy as np
import scipy.io.wavfile

connection = serial.Serial(port="COM3", baudrate=115200)
connection.reset_input_buffer()

incoming_serial = connection.readline()
chunk = 1024

timeout = time.time() + (60 * .25)

# wf = wave.open('C:/Users/sky20/Desktop/serialrecording/sound2.wav', 'wb')
# wf.setnchannels(1)
# wf.setsampwidth(2)
# wf.setframerate(16000)

# while time.time() < timeout:
#     d = wf.writeframes(incoming_serial)

# wf.close()

# 8 bits per sample
sample_format = pyaudio.paUInt8
chanels = 1

# Record at 16000 samples per second
smpl_rt = 16000
seconds = 4
filename = 'C:/Users/sky20/Desktop/serialrecording/sound3.wav'

# Create an interface to PortAudio
pa = pyaudio.PyAudio()

stream = pa.open(format=sample_format, channels=chanels,
                 rate=smpl_rt, input=True,
                 frames_per_buffer=chunk)

print('Recording...')

# Initialize array that be used for storing frames
frames = []

# Store data in chunks for 8 seconds
for i in range(0, int(smpl_rt / chunk * seconds)):
    data = stream.read(chunk)
    frames.append(data)

# Stop and close the stream
stream.stop_stream()
stream.close()

# Terminate - PortAudio interface
pa.terminate()

print('Done !!! ')

# Save the recorded data in a .wav format
sf = wave.open(filename, 'wb')
sf.setnchannels(chanels)
sf.setsampwidth(pa.get_sample_size(sample_format))
sf.setframerate(smpl_rt)
sf.writeframes(b''.join(frames))
sf.close()
