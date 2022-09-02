"""
Creating an SVM classifier based on two (directory) inputs
Showing the feature extraction more extensively than DemoSVMFeatureWrapping.py
Generate graphs on "spectral_centroid_mean" vs "spectral_entropy_mean"
"""

from pyAudioAnalysis import MidTermFeatures as aF
import os
import numpy as np
from sklearn.svm import SVC
import plotly.graph_objs as go
import plotly

# dirs: directory where the files to train the SVM are located
# Amy & Scott
# dirs = ["speakers/Amy", "speakers/Scott"]

# Amy & Matthew
# dirs = ["speakers/Amy", "speakers/Matthew"]

# Scott & Matthew
dirs = ["speakers/Matthew", "speakers/Scott"]

class_names = [os.path.basename(d) for d in dirs]  # set class names to be the folder titles

# setting the midterm steps/windows and short-term steps/windows
m_win, m_step, s_win, s_step = 0.5, 0.05, 0.025, 0.05  # in seconds
# mid-window and mid-step = 0.5 s and 0.05 s respectively
# short-window and short-step = 0.1 s and 0.05 s respectively

# feature extraction
features = []
for d in dirs:
    f, files, fn = aF.directory_feature_extraction(d, m_win, m_step, s_win, s_step)
    features.append(f)  # add to the features array

# select 2 features: 'spectral_centroid_mean' & 'spectral_entropy_mean'
# we select these two features because they are ideal for vocal analysis
# create feature matrices for the two classes
f1 = np.array([features[0][:, fn.index("spectral_centroid_mean")],
               features[0][:, fn.index("spectral_entropy_mean")]])
f2 = np.array([features[1][:, fn.index("spectral_centroid_mean")],
               features[1][:, fn.index("spectral_entropy_mean")]])

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
                     yaxis=dict(title="spectral_entropy_mean"))

# Concatenate arrays of zeros with the size of f1, f2, and f3
y = np.concatenate((np.zeros(f1.shape[1]), np.ones(f2.shape[1])))
# Concatenate the transpose of f1, f2
f = np.concatenate((f1.T, f2.T), axis=0)

# training the support vector machine
# Using supervised learning with support vector machines (SVM)
# SVC is from sklearn.svm
# kernel="rbf"  radial basis function
# popular in use with SVM classification
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
