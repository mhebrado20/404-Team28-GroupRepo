README

# TODO implement a main .py that executes all of these functions

Important* If using pc or pi as host computer make sure the dependencies in requirements.txt are installed
do this by navigating to the directory of requirements.txt and running the following command

py -m pip install -r requirements.txt (windows cmd)


When something needs to be "run" use a text editor w/ compiler such as Pycharm
How to use:
1) Navigate to the directory that has audio files that need to be converted to WAV
and run the ConvertToWav.py from the root directory

2) Run DemoAudioSplit.py in order to create the splits of both the training and test files
Splits are currently set to 0.5 sec

3) Run DemoFeatureWrapping.py to create the  gradient boost classification file -- THIS MAY TAKE A WHILE

4) Run DemoSingleClassification.py to test the incoming test file from the landline against the
classification model of the whitelisted/blacklisted voices

5) The highest classification value is returned and sent to the database using Upload_Data.py and then to the device connected to
the handset and the LEDs will glow green if there was a match on the whitelist