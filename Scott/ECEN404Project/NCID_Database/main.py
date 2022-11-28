import services.upload_data as ud
import services.calls as calls
import services.JSON_data_transfer as jdt
import services.remove_silence as rs
import services.mongoengine_setup as mongo_setup

# mongo_setup.global_initialization allows mongoengine to interact with the database,
# it only needs to be called once, but it must be done before using mongo engine to interact with any data

connection = mongo_setup.global_initialization()

# ui.run()  # This executes the run() function from user_interface.py

# to be triggered by recording variable sent by esp32
jdt.record_audio('COM3', 'C:/Users/sky20/Desktop/serialrecording')
# to be triggered by end of call upload folder variable (on-hook)
# ud.upload_folder('C:/Users/sky20/Desktop/serialrecording',
#                  'C:/Users/sky20/Desktop/404-Team28-GroupRepo/Matthew/voiceMap/speakers/filesToTest', 'recording')

# ud.retrieve_recording()

# jdt.ncid_msg_print(64)
# jdt.ncid_msg_print(75.1)
# jdt.ncid_msg_print(86)

"""
samplerate = 44100
fs = 100

t = np.linspace(0., 1., samplerate)

amplitude = np.iinfo(np.int16).max

data = amplitude * np.sin(2. * np.pi * fs * t)

write("C:/Users/sky20/Desktop/serialrecording/example.wav", samplerate, data.astype(np.int16))
"""
