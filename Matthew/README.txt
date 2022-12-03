README

# TODO implement a main .py that executes all of these functions

How to use:
1) Navigate to the directory that has audio files that need to be converted to WAV
and run the ConvertToWav.py from the root directory

2) Run DemoAudioSplit.py in order to create the splits of both the training and test files
Splits are currently set to 0.5 sec

3) Run DemoFeatureWrapping.py to create the  gradient boost classification file -- THIS MAY TAKE A WHILE

4) Run DemoSingleClassification.py to test the incoming test file from the landline against the
classification model of the whitelisted/blacklisted voices

5) The highest classification value is returned and sent to the database and then to the device connected to
the handset and the LEDs will glow green if there was a match on the whitelist