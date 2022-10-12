import services.upload_data as ud
import services.mongoengine_setup as mongo_setup

# mongo_setup.global_initialization allows mongoengine to interact with the database,
# it only needs to be called once, but it must be done before using mongo engine to interact with any data

mongo_setup.global_initialization()

# test call object
# call1 = Call()
# call1.upload_recording('C:/Users/sky20/Desktop/recordings/Speaker_0006/Speaker_0006_00001.wav', 'Recording0001')
ud.upload_folder('C:/Users/sky20/Desktop/recordings/Speaker_0006', 'Speaker_0006_00')
# ud.retrieve_recording("C:/Users/sky20/Desktop/retrievedfromdb", "Speaker_0002_002")

# ui.run()  # This executes the run() function from user_interface.py

