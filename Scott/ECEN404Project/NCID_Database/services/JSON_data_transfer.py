import json
import serial
import wave
import pyaudio
from upload_data import num_files


def record_audio(port: str, filelocation: str, samplerate: int = 9600, chunk: int = 1024, baudrate: int = 115200):
    connection = serial.Serial(port=port, baudrate=baudrate)
    connection.reset_input_buffer()

    # 16 bits per sample
    sample_format = pyaudio.paInt16
    chans = 1

    # Record at 9600 samples per second
    smpl_rt = samplerate
    seconds = 6
    filename = filelocation + '/recording' + str(num_files(filelocation)) + '.wav'
    # Create an interface to PortAudio
    pa = pyaudio.PyAudio()

    stream = pa.open(format=sample_format, channels=chans,
                     rate=smpl_rt, input=True, frames_per_buffer=chunk)

    print('recording')

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

    # Save the recorded data in a .wav format
    sf = wave.open(filename, 'wb')
    sf.setnchannels(chans)
    sf.setsampwidth(pa.get_sample_size(sample_format))
    sf.setframerate(smpl_rt)
    sf.writeframes(b''.join(frames))
    sf.close()


record_audio('COM3', 'C:/Users/sky20/Desktop/serialrecording')
