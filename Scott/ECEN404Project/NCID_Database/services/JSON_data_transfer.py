import json
import serial
import wave
import pyaudio
from Scott.ECEN404Project.NCID_Database.services.upload_data import num_files
from datetime import datetime
from datetime import date


def record_audio(port: str, filelocation: str, samplerate: int = 9600, chunk: int = 1024,
                 baudrate: int = 115200):
    connection = serial.Serial(port=port, baudrate=baudrate)
    connection.reset_input_buffer()

    # 16 bits per sample
    sample_format = pyaudio.paInt16
    chans = 1

    # Record at 9600 samples per second
    smpl_rt = samplerate
    seconds = 45
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


# record_audio('COM3', 'C:/Users/sky20/Desktop/serialrecording')


def ncid_msg_print(confidence_value: float, host="127.0.0.1", port="3334"):
    # confidence_value = this will be from matthews output code
    # TCPIP socket
    # 2ncid gateway clients
    # catch exceptions for improperly closed ports
    time_now = datetime.now()
    current_time = time_now.strftime("%H:%M")
    date_now = date.today()
    current_date = date_now.strftime("%m/%d/%y")
    if confidence_value > 75:
        print(r"MSG: <NCID-Defender has detected that the caller is on the NCID-Defender voicematch whitelist; ACCURACY:"
              + str(confidence_value) + "%> "
              r"###DATE*<" + current_date + ">"
              r"*TIME*<" + current_time + ">"
              r"*LINE*<->"
              "*NMBR\*<->\\"
              r"*MTYPE*SYS"
              r"*NAME*<NCID-Defender>*")
    elif confidence_value < 75:
        print(r"MSG: <NCID-Defender has detected that the caller is not on the NCID-Defender voicematch whitelist; ACCURACY:"
              + str(confidence_value) + "%> "
              r"###DATE*<" + current_date + ">"
              r"*TIME*<" + current_time + ">"
              r"*LINE*<->"
              "*NMBR\*<->\\"
              r"*MTYPE*SYS"
              r"*NAME*<NCID-Defender>*")
