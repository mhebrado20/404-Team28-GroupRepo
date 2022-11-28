"""
Does feature extraction and generates an SVM classification file
"""

from pyAudioAnalysis.audioTrainTest import extract_features_and_train

m_step, s_step = 0.05, 0.05  # set midterm-step and short-term step
# dirs: directory where the files to train the SVM are located
dirs = ["speakers/test/Benjamin_Netanyau", "speakers/test/Jens_Stoltenberg",
        "speakers/test/Julia_Gillard", "speakers/test/Magaret_Tarcher",
        "speakers/test/Nelson_Mandela"]

extract_features_and_train(dirs, m_step, m_step, s_step, s_step, "svm_rbf", "svm_dataset")
""" 
extract_features_and_train is a function from pyAudioAnalysis that handles feature extraction
and classifier straining. In my use case classifying Support Vector Machines (SVM) using 
Radial Basis Function kernel (RBF).

Parameters of extract_features_and_train are similar to that of directory_feature_extraction()
from DemoTrainingGraphs.py since they are needed for running the feature extraction code

The last two parameters are the type of classification and the name of the file to be generated

Once this code completes, a new file called "svm_all" will be created and will hold all the
classification data

Step through the process:
    1) Feature extraction: For each wav file in the directory one feature vector is extracted
    (hence the need for DemoAudioSplit.py). Runs calculations to generate feature vectors.

    2) Classifier evaluation: Create parameters based on the classifier type
    In our case "svm_rbf" is the classifier type and a parameter array.

    3) Generate the classifier file:
        Training: uses sklearn to create a trained SVM variable
        Generating the file: save file in the same directory that this .py file is in
"""