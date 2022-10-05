import services.upload_data as ud
import services.calls as calls
import services.JSON_data_transfer as jdt
import services.remove_silence as rs
import services.mongoengine_setup as mongo_setup

# mongo_setup.global_initialization allows mongoengine to interact with the database,
# it only needs to be called once, but it must be done before using mongo engine to interact with any data

connection = mongo_setup.global_initialization()

"""
# test call object
# call1 = Call()
# call1.upload_recording('C:/Users/sky20/Desktop/recordings/Speaker_0006/Speaker_0006_00001.wav', 'Recording0001')
# ud.upload_folder('C:/Users/sky20/Desktop/recordings/Speaker_0006', 'Speaker_0006_00')
# ud.retrieve_recording("C:/Users/sky20/Desktop/404-Team28-GroupRepo/Matthew/voiceMap/speakers/filesToTest",
#                      "Speaker_0006_001")

# ui.run()  # This executes the run() function from user_interface.py
"""

# begin recording the audio from the serial port
# probably need an if statement checking for a JSON packet containing 'start recording' packet info
# jdt.record_audio('COM3', 'C:/Users/sky20/Desktop/serialrecording')
# remove silence and upload to filesToTest directory
"""
ud.upload_folder('C:/Users/sky20/Desktop/serialrecording',
                 'C:/Users/sky20/Desktop/404-Team28-GroupRepo/Matthew/voiceMap/speakers/filesToTest', 'recording',
                 connection)
"""

jdt.ncid_msg_print(64)
jdt.ncid_msg_print(75.1)
jdt.ncid_msg_print(86)

