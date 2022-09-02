"""
Read the SVM classification file generated from DemoSVMFeatureWrapping.py (trained data)
and test new files against this trained data. Generate predictions for each test file and
a final average confidence interval
"""

from pyAudioAnalysis import audioTrainTest as aTT
import glob

# testDirectory: directory where the files to test the SVM are located
testDirectory = glob.glob("speakers/Amy/filesToTest/*.wav") + glob.glob("speakers/Scott/filesToTest/*.wav") \
                + glob.glob("speakers/Matthew/filesToTest/*.wav")

AmyAVG = 0
ScottAVG = 0
MatthewAVG = 0
i = 0


def numberOfFiles(directory):
    for k in directory:
        k += 1
    return k


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
    if i >= 0 and i < 43:
        AmyAVG += truncatePrecentage(p[0], 4)

    # Scott
    print(f"P({p_nam[1]} = {truncatePrecentage(p[1], 4)})")
    if i >= 43 and i < 97:
        ScottAVG += truncatePrecentage(p[1], 4)

    # Matthew
    print(f"P({p_nam[2]} = {truncatePrecentage(p[2], 4)})")
    if i >= 97 and i < 157:
        MatthewAVG += truncatePrecentage(p[2], 4)

    i += 1
    print()

# Calculate the averages
# Number of test files are known
print("\nCalculating Averages: ")
AmyAVG = AmyAVG / 42
ScottAVG = ScottAVG / 54
MatthewAVG = MatthewAVG / 60

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
