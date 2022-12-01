import json
import serial
import base64
import wave
import numpy as np
from scipy.io.wavfile import write
from Scott.ECEN404Project.NCID_Database.services.upload_data import num_files
from datetime import datetime
from datetime import date
import time
import matplotlib.pyplot as plt
import codecs


def record_audio(port: str, filelocation: str, samplerate: int = 16000, chunk: int = 1024,
                 baudrate: int = 256000):
    connection = serial.Serial(port=port, baudrate=baudrate)
    connection.reset_input_buffer()

    # 16 bits per sample
    chans = 1
    smpl_rt = samplerate
    filename = filelocation + '/recording' + str(num_files(filelocation)) + '.wav'

    print('recording\n')

    
    """
        This while loop records for a set amount of time set by the variable seconds, the intent is to read the data
        in from the serial port using numpy.frombuffer, appending this numpy.frombuffer array to the audio_data variable
        creating a large array containing the data of the audio recording in unsigned 8-bit PCM values
        
        This array will then be written to a file using the scipy.io write function which can handle numpy array data,
        this may need to be written as a raw file and converted using ffmpeg, but the functionality for converting has
        already been implemented by Matthew
    """
    print("Start initialize serial data")  # debug statement
    arr = np.array([])
    arr2 = np.array([])
    array = []
    n = 0
    data = []
    print("start loop")  # debug statement
    while True:
        print(connection.readline())
        if connection.readline() == '8':
            print("made it into if statement")
            while connection.readline() != '7':
                data = connection.readline()
                array.extend(data)
                # print(len(array))
            return

        # arr = np.fromiter((int(x, base=16) for x in data.split()), dtype=np.uint16)
        # data = np.frombuffer(connection.readline(), dtype=np.uint8, count=-1, offset=0, like=audio_data)
        # print("data initialize")
        # data = np.frombuffer(connection.read(1028), dtype=np.uint16, count=-1, offset=14, like=audio_data)
        # print(connection.readline().decode("base64"))
        # data = connection.readline()
        # print(connection.readline())
        # print(connection.read(20))
        # audio_data.append(data_array)

    # print(audio_data)

    # Save the recorded data in a .wav format
        # arr = np.append(arr, np.fromiter((int(x, 16) for x in data.split()), dtype=np.uint16))
    # print(array)
    arr = np.asarray(array)
    arr2 = np.frombuffer(arr, dtype=np.int16)
    array = arr2
    print(array)

    """
    sf = wave.open(filename, 'wb')
    sf.setnchannels(chans)
    sf.setsampwidth(1)
    sf.setframerate(16000)
    sf.writeframes(b''.join(array))
    sf.close()
    """

    print("done recording")  # debug statement


def ncid_msg_print(confidence_value: float, host="127.0.0.1", port="3334", line=""):
    # confidence_value = this will be from matthews output code
    # TCPIP socket
    # 2ncid gateway clients
    # catch exceptions for improperly closed ports
    time_now = datetime.now()
    current_time = time_now.strftime("%H:%M:%S")
    date_now = date.today()
    current_date = date_now.strftime("%y/%m/%d")
    if confidence_value > 75:
        print(r"MSG: <NCID-Defender has detected that the caller is on the NCID-Defender voicematch whitelist; "
              r"ACCURACY:"
              + str(confidence_value) + "%> "
              r"###DATE*<" + current_date + ">"
              r"*TIME*<" + current_time + ">"
              r"*LINE*<->"
              "*NMBR\*<->\\"
              r"*MTYPE*SYS"
              r"*NAME*<NCID-Defender>*")
    elif confidence_value < 75:
        print(r"MSG: <NCID-Defender has detected that the caller is not on the NCID-Defender voicematch whitelist; "
              r"ACCURACY:"
              + str(confidence_value) + "%> "
              r"###DATE*<" + current_date + ">"
              r"*TIME*<" + current_time + ">"
              r"*LINE*<->"
              "*NMBR\*<->\\"
              r"*MTYPE*SYS"
              r"*NAME*<NCID-Defender>*")
