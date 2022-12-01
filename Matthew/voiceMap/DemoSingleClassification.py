"""
Read the SVM classification file generated from DemoSVMFeatureWrapping.py (trained data)
and test new files against this trained data. Generate predictions for each test file and
a final average confidence interval
"""

from pyAudioAnalysis import audioTrainTest as aTT
import glob
from pathlib import Path
import os


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


def classify():
    testDirs = []

    for root, dirs, files in os.walk("speakers/filesToTest/"):
        for name in dirs:
            testDirs.append("speakers/filesToTest/" + name + "/")

    # testDirectory: directory where the files to test the SVM are located
    testDirectory = []
    for n in testDirs:
        testDirectory += glob.glob(n + "*.wav")

    numOfFiles = numFiles(testDirs[0])

    # perAVG number of variables is based on how many entries there are in white/blacklist separately
    per1 = "Amy"
    per2 = "Matthew"
    per3 = "Scott"
    perAVG1 = 0
    perAVG2 = 0
    perAVG3 = 0
    i = 0

    # function for truncating the percentages to display
    def truncatePrecentage(num, n):
        percent = int((num * 100) * (10 ** n) / (10 ** n))
        return float(percent)

    # for every file in the test directory
    for f in testDirectory:
        print(f"{f}:")
        c, p, p_nam = aTT.file_classification(f, "grad_all", "gradientboosting")  # orig svm_rbf

        print(f"P({p_nam[0]} = {truncatePrecentage(p[0], 4)})")
        if 0 < i <= numOfFiles:
            perAVG1 += truncatePrecentage(p[0], 4)
            print("AmyAVG: ", perAVG1)

        # Matthew
        print(f"P({p_nam[1]} = {truncatePrecentage(p[1], 4)})")
        if 0 < i <= numOfFiles:
            perAVG2 += truncatePrecentage(p[1], 4)
            print("MatthewAVG: ", perAVG2)

        # Scott
        print(f"P({p_nam[2]} = {truncatePrecentage(p[2], 4)})")
        if 0 < i <= numOfFiles:
            perAVG3 += truncatePrecentage(p[2], 4)
            print("ScottAVG: ", perAVG3)

        i += 1
        print()

    print("perAVG1: ", perAVG1)
    print("perAVG2: ", perAVG2)
    print("perAVG3: ", perAVG3)

    # Calculate the averages
    # Number of test files are known
    print("\nCalculating Averages: ")
    perAVG1 = perAVG1 / numOfFiles
    perAVG2 = perAVG2 / numOfFiles
    perAVG3 = perAVG3 / numOfFiles

    # Final print statements
    print("Testing against Amy's Files: ", perAVG1)
    print("Testing against Matthew's Files: ", perAVG2)
    print("Testing against Scott's Files: ", perAVG3)

    if perAVG1 > perAVG2 and perAVG1 > perAVG3:
        print("\nHighest match: ", per1)
        return perAVG1
    if perAVG2 > perAVG1 and perAVG2 > perAVG3:
        print("\nHighest match: ", per2)
        return perAVG2
    if perAVG3 > perAVG2 and perAVG3 > perAVG1:
        print("\nHighest match: ", per3)
        return perAVG3


# print("Highest Average: ", classify())
"""
file_classification looks for a file called "grad_all" with the classification type of
"gradientboost"
    converts the given file (f in test directory) to mono so it is easier to segment the
    features and conducts feature extractions on the files to test

Uses a classifier wrapper that returns a class ID R and prediction P
"""
