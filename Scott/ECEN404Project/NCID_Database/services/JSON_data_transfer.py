import json
import serial
import wave
import io
import numpy as np
import soundfile as sf
from scipy.io.wavfile import write

connection = serial.Serial(port="COM3", baudrate=115200)
connection.reset_input_buffer()

# open the file, name it and set it to write
audiofile = wave.open('C:/Users/sky20/Desktop/serialrecording/sound.wav', 'wb')
audiofile.setframerate(16000)  # sample rate
audiofile.setnchannels(1)  # mono(1) or stereo(2)
audiofile.setsampwidth(2)


def convert_bytearray_to_wav_ndarray(input_bytearray: bytes, sampling_rate=16000):
    bytes_wav = bytes()
    byte_io = io.BytesIO(bytes_wav)
    write(byte_io, sampling_rate, np.frombuffer(input_bytearray, dtype=np.uint8))
    output_wav = byte_io.read()
    output, samplerate = sf.read(io.BytesIO(output_wav))
    return output


while connection.isOpen():
    data = connection.readline()  # read the command line data
    audiofile = convert_bytearray_to_wav_ndarray(data)

scipy.io.wavfile.write('C:/Users/sky20/Desktop/serialrecording/sound.wav', 16000, audiofile)
audiofile.close()
