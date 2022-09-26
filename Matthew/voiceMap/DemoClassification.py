"""
Read the SVM classification file generated from DemoSVMFeatureWrapping.py (trained data)
and test new files against this trained data. Generate predictions for each test file and
a final average confidence interval
"""

from pyAudioAnalysis import audioTrainTest as aTT
import glob
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


# testDirectory: directory where the files to test the SVM are located
testDirectory = glob.glob("speakers/Amy/filesToTest/splits/*.wav") \
                + glob.glob("speakers/Scott/filesToTest/splits/*.wav") \
                + glob.glob("speakers/Matthew/filesToTest/splits/*.wav")

numAmy = numFiles("speakers/Amy/filesToTest/splits/")
numScott = numFiles("speakers/Scott/filesToTest/splits/")
numMatthew = numFiles("speakers/Matthew/filesToTest/splits/")

print("A: ", numAmy)
print("S: ", numScott)
print("M: ", numMatthew)

AmyAVG = 0
ScottAVG = 0
MatthewAVG = 0
i = 0


# function for truncating the percentages to display
def truncatePrecentage(num, n):
    percent = int((num * 100) * (10 ** n) / (10 ** n))
    return float(percent)


# for every file in the test directory
for f in testDirectory:
    print(f"{f}:")
    c, p, p_nam = aTT.file_classification(f, "svm_all", "svm_rbf")

    # Amy
    print(f"P({p_nam[0]} = {truncatePrecentage(p[0], 4)})")
    if 0 <= i < numAmy:
        AmyAVG += truncatePrecentage(p[0], 4)

    # Scott
    print(f"P({p_nam[1]} = {truncatePrecentage(p[1], 4)})")
    if numAmy <= i < numAmy + numScott:
        ScottAVG += truncatePrecentage(p[1], 4)

    # Matthew
    print(f"P({p_nam[2]} = {truncatePrecentage(p[2], 4)})")
    if numAmy + numScott <= i < numAmy + numScott + numMatthew:
        MatthewAVG += truncatePrecentage(p[2], 4)

    i += 1
    print()

# Calculate the averages
# Number of test files are known
print("\nCalculating Averages: ")
AmyAVG = AmyAVG / numAmy
ScottAVG = ScottAVG / numScott
MatthewAVG = MatthewAVG / numMatthew

# Final print statements
print("Testing Amy's Files: ", AmyAVG)
print("Testing Scott's Files: ", ScottAVG)
print("Testing Matthew's Files: ", MatthewAVG)

"""
file_classification looks for a file called "svm_all" with the classification type of
"svm_rbf"
    converts the given file (f in test directory) to mono so it is easier to segment the
    features and conducts feature extractions on the files to test
    
Uses a classifier wrapper that returns a class ID R and prediction P
"""
