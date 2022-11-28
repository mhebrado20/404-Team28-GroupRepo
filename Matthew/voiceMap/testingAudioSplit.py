# from pydub import AudioSegment
# import math
#
# dirs = "speakers/Amy/"
#
# splitAudio = AudioSegment.from_wav("Amy_Chen_Orig.wav")

from pydub import AudioSegment
import math
import glob

class AudioSplitter:
    # Set the initial values
    def __init__(self, dirs, filename):
        self.dirs = dirs  # ex case: "speakers/Amy/"
        self.filename = filename  # ex case: "Amy_Chen_Orig.wav"
        # self.filepath = dirs + '\\' + filename  # speakers/Amy/\\Amy_Chen_Orig.wav
        self.filepath = filename

        self.audio = AudioSegment.from_wav(self.filepath)  # using pydub to get the whole audio segment at filepath

    # returns the duration of the audio segment in seconds
    def getDuration(self):
        return self.audio.duration_seconds

    # generates a single split
    def singleSplit(self, from_sec, to_sec, split_filename):
        t1 = from_sec * 1000
        t2 = to_sec * 1000
        split_audio = self.audio[t1:t2]
        split_audio.export(self.dirs + '\\' + split_filename, format="wav")

    # calls single split for the given interval of split
    def multipleSplit(self, sec_per_split):
        total_sec = math.ceil(self.getDuration())
        for i in range(0, total_sec, sec_per_split):
            split_fn = str(i) + ".wav"  # + '_' + self.filename
            self.singleSplit(i, i + sec_per_split, split_fn)
            print(str(i) + ' Done')
            if i == total_sec - sec_per_split:
                print('All split successfully')


dirs = ["speakers/Amy/", "speakers/Scott/"]
testDirs = ["speakers/Amy/filesToTest/", "speakers/Scott/filesToTest/"]
# file = glob.glob("*.wav")

# generate files to train SVM
for d in dirs:
    # print("\nd: ", d)
    file = glob.glob(d + "*.wav")
    # print("\n file: ", file)
    for f in file:
        # print("\nf: ", f)
        split_wav = AudioSplitter(d, f)
        split_wav.multipleSplit(sec_per_split=1)

# generate files to test SVM
for d in testDirs:
    print("\nd: ", d)
    file = glob.glob(d + "*.wav")
    print("\n file: ", file)
    for f in file:
        print("\nf: ", f)
        split_wav = AudioSplitter(d, f)
        split_wav.multipleSplit(sec_per_split=1)
