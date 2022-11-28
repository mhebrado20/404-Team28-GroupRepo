### Using pyAudioAnalysis to open a directory of WAV files
### and classify them based on content

from pyAudioAnalysis import MidTermFeatures as aF
import os
import numpy as np
import plotly.graph_objs as go
import plotly

# adding the directory on where to find the files
# --only doing 2 of the test to start--
dirs = ["speakers/test/Benjamin_Netanyau", "speakers/test/Nelson_Mandela"]
class_names = [os.path.basename(d) for d in dirs]  # set class names to be the folder titles

# setting the mid-term steps/windows and short-term steps/windows
m_win, m_step, s_win, s_step = 1, 1, 0.1, 0.05
# mid window and mid step = 1 s
# short window and short step = 0.1 s and 0.05 s respectively

# feature extraction
features = []  # create an empty array that we will populate with output from feature extraction
for d in dirs:  # extract features for each directory
    f, files, fn = aF.directory_feature_extraction(d, m_win, m_step, s_win, s_step)
    features.append(f)  # add to the features array

# each file in the directory has a length of 1 sec
# there are 1500 samples of each speaker (classes)
# a matrix of order [1500 x 138] is expected
# 138 comes from the number of segment features
print(features[0].shape, features[1].shape)  # generates 2 1500 x 138 matrices

# select 2 features: 'spectral_centroid_mean' & 'energy_entropy_mean'
# create feature matrices for the two classes
f1 = np.array([features[0][:, fn.index("spectral_centroid_mean")],
               features[0][:, fn.index("spectral_entropy_mean")]])
f2 = np.array([features[1][:, fn.index("spectral_centroid_mean")],
               features[1][:, fn.index("spectral_entropy_mean")]])

# plotting the features
plots = [go.Scatter(x=f1[0, :], y=f1[1, :],
                    name=class_names[0], mode="markers"),
         go.Scatter(x=f2[0, :], y=f2[1, :],
                    name=class_names[1], mode="markers")]
mylayout = go.Layout(xaxis=dict(title="spectral_centroid_mean"),
                     yaxis=dict(title="spectral_entropy_mean"))

plotly.offline.iplot(go.Figure(data=plots, layout=mylayout))

### NOTES: Using only two features 'spectral_centroid_mean' and
###        'energy_entropy_mean' pyAA was able to generate a scatterplot
###        categorizing the data
