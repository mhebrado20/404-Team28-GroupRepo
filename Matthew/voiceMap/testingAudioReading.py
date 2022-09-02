"""
Do short-term feature extraction on a single audio file
"""
from pyAudioAnalysis import ShortTermFeatures as aF
from pyAudioAnalysis import audioBasicIO as aIO
import numpy as np
import plotly.graph_objs as go
import plotly
import IPython

# read audio from dir
# fs - sampling rate
# s  - signal
fs, s = aIO.read_audio_file("speakers/test/Benjamin_Netanyau/1.wav")
# return the sampling rate and signal as a numpy array

IPython.display.display(IPython.display.Audio("speakers/test/Benjamin_Netanyau/1.wav"))

# print length in seconds
duration = len(s) / float(fs)

print(f"duration = {duration} seconds\n")

# extracting short-term features using 50ms window; step of 50ms
win, step = 0.050, 0.050
[f, fn] = aF.feature_extraction(s, fs, int(fs * win), int(fs * step))

# print results
# the number of short-term features that are generated is based on
print(f"{f.shape[1]} frames, {f.shape[0]} short-term featuers")
print("Feature names: ")

for i, nam in enumerate(fn):
    print(f"{i}:{nam}")

# plot short-term energy
# create time axis in seconds
time = np.arange(0, duration - step, win)

# get feature with name "energy"
energy = f[fn.index("energy"), :]
mylayout = go.Layout(yaxis=dict(title="frame energy value"),
                     xaxis=dict(title="time (s)"))

# generate plot
plotly.offline.iplot(go.Figure(layout=mylayout,
                               data=[go.Scatter(x=time, y=energy)]))
