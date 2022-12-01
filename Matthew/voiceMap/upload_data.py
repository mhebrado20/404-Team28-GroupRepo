import json
import serial
import base64
import wave
import numpy as np
from scipy.io.wavfile import write
from datetime import datetime
from datetime import date
import time
import matplotlib.pyplot as plt
from Matthew.voiceMap.DemoSingleClassification import *
import codecs

"""
class Decoder(json.JSONDecoder):
    def decode(self, s):
        result = super().decode(s)  # result = super(Decoder, self).decode(s) for Python 2.x
        return self._decode(result)

    def _decode(self, o):
        if isinstance(o, str):
            try:
                return int(o)
            except ValueError:
                return o
        elif isinstance(o, dict):
            return {k: self._decode(v) for k, v in o.items()}
        elif isinstance(o, list):
            return [self._decode(v) for v in o]
        else:
            return o
"""

def num_files(folderLocation):
    # define our return variable, this one counts the number of files in a folder, so it is called quantity, start at
    # one to prevent counting errors; done by trial and error, I was always one file short on my quantity value
    n = 1
    # this will use the .iterdir() function of path to list the subdirectories of the folder_location path
    for p in Path(folderLocation).iterdir():
        # if the subdirectory is a file then increment quantity
        if p.is_file():
            n += 1

    return n

def record_audio(port: str, filelocation: str, samplerate: int = 16000, chunk: int = 1024,
                 baudrate: int = 115200):
    connection = serial.Serial(port=port, baudrate=baudrate)
    connection.reset_input_buffer()

    # 16 bits per sample
    chans = 1

    """
    # time for name
    time_now = datetime.now()
    current_time = time_now.strftime("%H:%M:%S")
    date_now = date.today()
    current_date = date_now.strftime("%y/%m/%d_")
    complete_time = current_date + current_time
    """

    smpl_rt = samplerate
    seconds = 10
    filename = filelocation + '/recording' + str(num_files(filelocation)) + '.wav'
    # Create an interface to PortAudio
    # pa = pyaudio.PyAudio()

    print('recording\n')

    # Initialize array that be used for storing frames
    # frames = []

    # Store data in chunks for 8 seconds
    # print("int(smpl_rt / chunk * seconds)", int(smpl_rt / chunk * seconds))
    timeout_start = time.time()

    audio_data = np.array([], dtype=np.uint16, ndmin=1)
    # data = np.array([], dtype=np.uint8)

    """
        This while loop records for a set amount of time set by the variable seconds, the intent is to read the data
        in from the serial port using numpy.frombuffer, appending this numpy.frombuffer array to the audio_data variable
        creating a large array containing the data of the audio recording in unsigned 8-bit PCM values

        This array will then be written to a file using the scipy.io write function which can handle numpy array data,
        this may need to be written as a raw file and converted using ffmpeg, but the functionality for converting has
        already been implemented by Matthew
    """
    print("Start initialize serial data")
    arr = np.array([])
    arr2 = np.array([])
    array = []
    n = 0
    data = []
    while time.time() < timeout_start + seconds:
        # connection.reset_input_buffer()
        data = connection.readline().split()
        if n > 1:
            array.extend(data)
            print(len(array))
        n += 1
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

    plt.rcParams["figure.figsize"] = [7.50, 3.50]
    plt.rcParams["figure.autolayout"] = True

    x = arr2
    y = np.sort(x)

    plt.title("Line graph")
    plt.plot(x, y, color="red")

    plt.show()

    """
    sf = wave.open(filename, 'wb')
    sf.setnchannels(chans)
    sf.setsampwidth(1)
    sf.setframerate(16000)
    sf.writeframes(b''.join(array))
    sf.close()
    """

    print("done recording")


# record_audio('COM3', 'C:/Users/sky20/Desktop/serialrecording')


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
        print(
            r"MSG: <NCID-Defender has detected that the caller is on the NCID-Defender voicematch whitelist; ACCURACY:"
            + str(confidence_value) + "%> "
                                      r"###DATE*<" + current_date + ">"
                                                                    r"*TIME*<" + current_time + ">"
                                                                                                r"*LINE*<->"
                                                                                                "*NMBR\*<->\\"
                                                                                                r"*MTYPE*SYS"
                                                                                                r"*NAME*<NCID-Defender>*")
    elif confidence_value < 75:
        print(
            r"MSG: <NCID-Defender has detected that the caller is not on the NCID-Defender voicematch whitelist; ACCURACY:"
            + str(confidence_value) + "%> "
                                      r"###DATE*<" + current_date + ">"
                                                                    r"*TIME*<" + current_time + ">"
                                                                                                r"*LINE*<->"
                                                                                                "*NMBR\*<->\\"
                                                                                                r"*MTYPE*SYS"
                                                                                                r"*NAME*<NCID-Defender>*")


ncid_msg_print(classify())
