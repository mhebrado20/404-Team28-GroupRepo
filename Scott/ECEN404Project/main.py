import mongoengine

import user_interface as ui
import src.data.mongoengine_setup as mongo_setup
import src.services.upload_data as ud

from data.calls import Call

# mongo_setup.global_initialization allows mongoengine to interact with the database,
# it only needs to be called once, but it must be done before using mongo engine to interact with any data

mongo_setup.global_initialization()

# test call object
# call1 = Call()
# call1.upload_recording('C:/Users/sky20/Desktop/pythonProject2/voice signature dataset/archive/50_speakers_audio_data
# /Speaker_0000/Speaker_0000_00001.wav', 'Recording0001')
# ud.upload_folder('C:/Users/sky20/Desktop/pythonProject2/voice signature dataset/archive/50_speakers_audio_data
# /Speaker_0000', 'Speaker_0000_000', 93)
# ud.retrieve_recording("C:/Users/sky20/Desktop/retrievedfromdb", "Speaker_0002_002")

ui.run()  # This executes the run() function from user_interface.py

