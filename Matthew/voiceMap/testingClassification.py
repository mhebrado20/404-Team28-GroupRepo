"""
Read the SVM classification file generated from DemoSVMFeatureWrapping.py (trained data)
and test new files against this trained data. Generate predictions for each test file and
a final average confidence interval
"""

from pyAudioAnalysis import audioTrainTest as aTT
import glob

# testDirectory: directory where the files to test the SVM are located
testDirectory = glob.glob("speakers/test/testFiles/bny/*.wav") + glob.glob("speakers/test/testFiles/jsb/*.wav") \
                + glob.glob("speakers/test/testFiles/jgd/*.wav") + glob.glob("speakers/test/testFiles/mtr/*.wav") \
                + glob.glob("speakers/test/testFiles/nmd/*.wav")

NetanyauAVG = 0
StoltenbergAVG = 0
GillardAVG = 0
TarcherAVG = 0
MandelaAVG = 0
i = 0


# function for truncating the percentages to display
def truncatePrecentage(num, n):
    percent = int((num * 100) * (10 ** n) / (10 ** n))
    return float(percent)


# print(truncatePrecentage(192.67, 4))  # test print
# for every file in the test directory
for f in testDirectory:
    print(f"{f}:")
    c, p, p_nam = aTT.file_classification(f, "svm_dataset", "svm_rbf")

    # Netanyau
    print(f"P({p_nam[0]} = {truncatePrecentage(p[0], 4)})")
    if i >= 0 and i < 102:
        NetanyauAVG += truncatePrecentage(p[0], 4)

    # Stoltenberg
    print(f"P({p_nam[1]} = {truncatePrecentage(p[1], 4)})")
    if i >= 102 and i < 203:
        StoltenbergAVG += truncatePrecentage(p[1], 4)

    # Gillard
    print(f"P({p_nam[2]} = {truncatePrecentage(p[2], 4)})")
    if i >= 203 and i < 304:
        GillardAVG += truncatePrecentage(p[2], 4)

    # Tarcher
    print(f"P({p_nam[3]} = {truncatePrecentage(p[3], 4)})")
    if i >= 304 and i < 405:
        TarcherAVG += truncatePrecentage(p[3], 4)

    # Mandela
    print(f"P({p_nam[4]} = {truncatePrecentage(p[4], 4)})")
    if i >= 405 and i < 506:
        MandelaAVG += truncatePrecentage(p[4], 4)

    i += 1
    print()

# Calculate the averages
# Number of test files are known
print("\nCalculating Averages: ")
NetanyauAVG = NetanyauAVG / 101
StoltenbergAVG = StoltenbergAVG / 101
GillardAVG = GillardAVG / 101
TarcherAVG = TarcherAVG / 101
MandelaAVG = MandelaAVG / 101

# Final print statements
print("Testing Netanyau's Files: ", NetanyauAVG)
print("Testing Stoltenberg's Files: ", StoltenbergAVG)
print("Testing Gillard's Files: ", GillardAVG)
print("Testing Tarcher's Files: ", TarcherAVG)
print("Testing Mandela's Files: ", MandelaAVG)
