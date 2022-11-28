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

testDirs = []

for root, dirs, files in os.walk("speakers/filesToTest/"):
    for name in dirs:
        testDirs.append("speakers/filesToTest/" + name + "/")

# testDirectory: directory where the files to test the SVM are located
testDirectory = []
for n in testDirs:
    testDirectory += glob.glob(n + "*.wav")

numAmy = numFiles(testDirs[0])
numMatthew = numFiles(testDirs[1])
numScott = numFiles(testDirs[2])

AmyAVG = 0
MatthewAVG = 0
ScottAVG = 0
i = 0


# function for truncating the percentages to display
def truncatePrecentage(num, n):
    percent = int((num * 100) * (10 ** n) / (10 ** n))
    return float(percent)


# for every file in the test directory
for f in testDirectory:
    print(f"{f}:")
    c, p, p_nam = aTT.file_classification(f, "grad_all", "gradientboosting")  # orig svm_rbf

    # Amy
    print(f"P({p_nam[0]} = {truncatePrecentage(p[0], 4)})")
    if 0 < i <= numAmy:
        AmyAVG += truncatePrecentage(p[0], 4)
        print("AmyAVG: ", AmyAVG)

    # Matthew
    print(f"P({p_nam[1]} = {truncatePrecentage(p[1], 4)})")
    if numAmy < i <= (numAmy + numMatthew):
        MatthewAVG += truncatePrecentage(p[1], 4)
        print("MatthewAVG: ", MatthewAVG)

    # Scott
    print(f"P({p_nam[2]} = {truncatePrecentage(p[2], 4)})")
    if (numAmy + numMatthew) < i <= (numAmy + numMatthew + numScott):
        ScottAVG += truncatePrecentage(p[2], 4)
        print("ScottAVG: ", ScottAVG)

    i += 1
    print()

print("AmyAVG: ", AmyAVG)
print("MatthewAVG: ", MatthewAVG)
print("ScottAVG: ", ScottAVG)

# Calculate the averages
# Number of test files are known
print("\nCalculating Averages: ")
AmyAVG = AmyAVG / numAmy
MatthewAVG = MatthewAVG / numMatthew
ScottAVG = ScottAVG / numScott

print("A: ", numAmy)
print("M: ", numMatthew)
print("S: ", numScott)


# Final print statements
print("Testing Amy's Files: ", AmyAVG)
print("Testing Matthew's Files: ", MatthewAVG)
print("Testing Scott's Files: ", ScottAVG)

"""
file_classification looks for a file called "grad_all" with the classification type of
"gradientboost"
    converts the given file (f in test directory) to mono so it is easier to segment the
    features and conducts feature extractions on the files to test

Uses a classifier wrapper that returns a class ID R and prediction P
"""
