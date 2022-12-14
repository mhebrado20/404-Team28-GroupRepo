"""
Given a file path of a .wav file this code will split
the given .wav file into [variable] second intervals
and generate new .wav files containing those splits

example: given a 3 minute .wav, wanting 1-second intervals
output: 180 new .wav files will be generated
each with the file name 0.wav . . . 179.wav
"""

from pydub import AudioSegment
import math
import glob
import numpy as np
from pathlib import Path

def numFiles(folderLocation):
    # define our return variable, this one counts the number of files in a folder, so it is called quantity, start at
    # one to prevent counting errors; done by trial and error, I was always one file short on my quantity value
    n = 0
    # this will use the .iterdir() function of path to list the subdirectories of the folder_location path
    for p in Path(folderLocation).iterdir():
        # if the subdirectory is a file then increment quantity
        if p.is_file():
            n += 1

    return n

class AudioSplitter:
    # Set the initial values
    def __init__(self, dirs, filename, n):
        self.dirs = dirs  # declare the directory that the file is in
        self.filename = filename  # res_str  # parameter filename is the same as the full filepath
        self.filepath = dirs + "splits" + str(n) + "/"
        self.audio = AudioSegment.from_wav(
            self.dirs + self.filename)  # using pydub to get the whole audio segment at filepath

    # returns the duration of the audio segment in seconds
    def getDuration(self):
        return self.audio.duration_seconds

    # generates a single split
    def singleSplit(self, from_sec, to_sec, split_filename):
        t1 = from_sec * 1000  # millisecond conversion
        t2 = to_sec * 1000
        # print("filepath: " + self.filepath)
        split_audio = self.audio[t1:t2]  # get a time interval from t1 and t2
        for n in range(numFiles(self.dirs)):
            Path(self.filepath).mkdir(parents=True, exist_ok=True)
            split_audio.export(self.filepath + '\\' + split_filename, format="wav")
        # export that split as a .wav file with the name of split_filename
        # sent to it from the function call

    # calls single split and sends it data on how long each split should be
    def multipleSplit(self, sec_per_split):
        total_sec = math.ceil(self.getDuration())  # gets the total duration of the audio segment as a whole integer
        # loop starts at 0 until the full duration (s) in intervals of sec_per_split
        for i in np.arange(0, total_sec - 0.5, sec_per_split):
            split_fn = str(i) + ".wav"  # define naming sense for the file -- 0.wav . . . 999.wav etc
            self.singleSplit(i, i + sec_per_split, split_fn)  # call single split to create a single interval
            print(str(i) + ' Done')
            if i == total_sec - sec_per_split:  # stopping condition
                print('All split successfully')


dirs = ["speakers/Amy/", "speakers/Matthew/", "speakers/Scott/"]
testDirs = ["speakers/filesToTest/"]


# function for generating the files where d is the filepath from dirs or testDirs
def genFiles(d):
    # print("\nd: ", d)  # test prints
    # glob.glob() function creates a list of all files in that directory that end in .wav
    file = glob.glob(d + "*.wav")
    n = 1
    # the reason I chose to do it with glob.glob() is for use in 404 integration; when a user uploads a file to a
    # directory, they can just select the directory and the .wav file won't need to be called anything specific
    # as long as its placed in the right directory it will automatically find the file and split it up

    for f in file:  # loop for each file in the directory where f is the exact filepath
        split_word = "\\"
        res_str = f.split(split_word)[1]
        # print("\nf: ", res_str)
        split_wav = AudioSplitter(d, res_str, n)  # call the AudioSplitter class with d (directory) and
        # f (exact filepath)
        split_wav.multipleSplit(sec_per_split=0.5)  # call multipleSplit to split the audio segment on
        # 0.5 second intervals
        n += 1


# generate files to train SVM
for d in dirs:
    genFiles(d)  # generate files in each directory given

# generate files to test SVM
for d in testDirs:
    genFiles(d)  # generate files in each directory given
