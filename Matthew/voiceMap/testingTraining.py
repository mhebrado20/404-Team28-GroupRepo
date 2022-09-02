### Similar set up to testingDirectoryFE.py

# training an SVM classifier
# drawing respective decision surfaces

from pyAudioAnalysis import MidTermFeatures as aF
import os
import numpy as np
from sklearn.svm import SVC
import plotly.graph_objs as go
import plotly

# adding the directory on where to find the files
# only doing 2 of the test to start
dirs = ["speakers/test/Benjamin_Netanyau", "speakers/test/Nelson_Mandela"]  # successful run!
# dirs = ["speakers/test/Jens_Stoltenberg", "speakers/test/Magaret_Tarcher"]  # successful run!
# testing with 3 speakers
# I can't figure out how to make it work with 3
# dirs = ["speakers/test/Benjamin_Netanyau", "speakers/test/Nelson_Mandela", "speakers/test/Jens_Stoltenberg"]
# a potential problem case
# dirs = ["speakers/test/Benjamin_Netanyau", "speakers/test/Jens_Stoltenberg"]  # successful run!
# dirs = ["speakers/test/Nelson_Mandela", "speakers/test/Jens_Stoltenberg"]  # successful run!
class_names = [os.path.basename(d) for d in dirs]  # set class names to be the folder titles

# setting the mid-term steps/windows and short-term steps/windows
m_win, m_step, s_win, s_step = .5, 0.05, 0.025, 0.05  # original values
# m_win, m_step, s_win, s_step = 0.05, 0.05, 0.025, 0.005
# mid window and mid step = 1 s
# short window and short step = 0.1 s and 0.05 s respectively

# feature extraction
features = []
for d in dirs:
    f, files, fn = aF.directory_feature_extraction(d, m_win, m_step, s_win, s_step)
    features.append(f)  # add to the features array

# select 2 features: 'spectral_centroid_mean' & 'energy_entropy_mean' -- wrong
# create feature matrices for the two classes
f1 = np.array([features[0][:, fn.index("spectral_centroid_mean")],
               features[0][:, fn.index("spectral_entropy_mean")]])  # was energy_entropy_mean
f2 = np.array([features[1][:, fn.index("spectral_centroid_mean")],
               features[1][:, fn.index("spectral_entropy_mean")]])  # was energy_entropy_mean

# plotting the features
p1 = go.Scatter(x=f1[0, :], y=f1[1, :],
                name=class_names[0],
                marker=dict(size=10, color="rgba(255,182,193,0.9)"),
                mode="markers")
p2 = go.Scatter(x=f2[0, :], y=f2[1, :],
                name=class_names[1],
                marker=dict(size=10, color="rgba(100,100,220,0.9)"),
                mode="markers")

mylayout = go.Layout(xaxis=dict(title="spectral_centroid_mean"),
                     yaxis=dict(title="spectral_entropy_mean"))  # was energy_entropy_mean

y = np.concatenate((np.zeros(f1.shape[1]), np.ones(f2.shape[1])))
f = np.concatenate((f1.T, f2.T), axis=0)

# training the support vector machine
cl = SVC(kernel="rbf", C=20)
cl.fit(f, y)

# apply the trained model on the grid
x_ = np.arange(f[:, 0].min(), f[:, 0].max(), 0.002)
y_ = np.arange(f[:, 1].min(), f[:, 1].max(), 0.002)

xx, yy = np.meshgrid(x_, y_)

Z = cl.predict(np.c_[xx.ravel(), yy.ravel()]).reshape(xx.shape) / 2

# visualize on the grid on the plot (decision surfaces)
cs = go.Heatmap(x=x_, y=y_, z=Z, showscale=False,
                colorscale=[[0, "rgba(255,182,192,0.3)"],
                            [1, "rgba(100,100,220,0.3)"]])

mylayout = go.Layout(xaxis=dict(title="spectral_centroid_mean"),
                     yaxis=dict(title="spectral_entropy_mean"))  # was energy_entropy_mean

# generate plot
plotly.offline.iplot(go.Figure(data=[p1, p2, cs], layout=mylayout))

### NOTES Can only test 2 at a time.

