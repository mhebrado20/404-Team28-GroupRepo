from Scott.ECEN404Project.NCID_Database.services.calls import Call
import Scott.ECEN404Project.NCID_Database.services.obtain_call_length as ocl
from Scott.ECEN404Project.NCID_Database.services.remove_silence import remove_silence
from pathlib import Path


# from data.incoming_callers import IncomingCaller
# from data.receiving_callers import ReceivingCaller
# from data.recordings import Recording
# the above imports will be used in ECEN404

# function to convert the audio information into an intuitively readable time
# this may need to be implemented in the calls class


def retrieve_recording(file_location: str, name: str, database='NCID_local_DB', host='127.0.0.1'):
    # Find the first object in database with file_name=name
    data = Call.objects(file_name=name).first()
    # error check
    if data is None:
        print("This file does not exist in the database, please try again.")
        return
    # Now we will read the contents of that object into a variable
    output_file = data.call_recording.read()
    # We will then open the file location we wish to retrieve the file to and name the file
    output = open(file_location + "/" + name + ".wav", "wb")
    # Write the data that we have read into the output_file variable into that location
    output.write(output_file)
    # Close the file as we are now done writing
    output.close()
    # Save the object in the database ** this step may be unnecessary, but I have limited knowledge with databases,
    # so I wanted to be safe
    data.save()


def delete_local_recordings(folder_location: str, name: str):
    # Warn the user that the file will be deleted
    print("WARNING: This file will be deleted from local storage")
    # Get confirmation from user that the file will be deleted
    delete_recording = input("Would you like to delete this file? (y, n): ")
    # If confirmed continue with deletion
    if delete_recording.lower() == 'y':
        # Take the user input path and use it to unlink the file using pathlib
        Path(folder_location + "/" + name + ".wav").unlink()
        # Notify the user that the file has been deleted
        print("Deleting file...")
        print("File has been deleted")
    print("\n")


def delete_database_recordings(name: str, database='NCID_local_DB', host='127.0.0.1'):
    # Warning for end user
    print("WARNING: This file will be deleted from database storage")
    # Get input for file deletion confirmation
    delete_recording = input("Would you like to delete this file? (y, n): ")
    # If yes then proceed with delete
    if delete_recording.lower() == 'y':
        # Assign database object to variable, this is only the first object by this name, if there are multiple objects
        # by the same name only the first will be deleted, if full object deletion is wanted then the code must be
        # slightly altered
        file_to_delete = Call.objects(file_name=name).first()
        # Make sure the file was actually retrieved, if it does not exist in the database the variable will be type
        # NoneType, so we check if the variable is NoneType here
        if file_to_delete is None:
            print("This file does not exist in the database, please try again.")
            return
        # If the file exists delete the file from the database
        file_to_delete.delete()
        # Let the user know it has been deleted
        print("Deleting file...")
        print("File has been deleted")
    print("\n")


def error_check(folder_location, file_name):
    folder_location_exists = Path(folder_location).exists()
    # WARNING this will only error check the given dataset file naming convention until around speaker26
    # in the 404 implementation of this the naming convention will be consistent across all datasets
    # this checks only the first file to see if it exists as another function will automatically check how many files
    # are in the directory given by folder_location
    file_location_exists = Path(folder_location + "/" + file_name + ".wav").exists()

    # boolean multiplication of the statements checked in order to make sure that both must be true to pass
    return folder_location_exists * file_location_exists


def error_check_file(folder_location, file_name):
    folder_location_exists = Path(folder_location).exists()
    # WARNING this will only error check the given dataset file naming convention until around speaker26
    # in the 404 implementation of this the naming convention will be consistent across all datasets
    # this checks only the first file to see if it exists as another function will automatically check how many files
    # are in the directory given by folder_location
    file_location_exists = Path(folder_location + "/" + file_name + ".wav").exists()

    # boolean multiplication of the statements checked in order to make sure that both must be true to pass
    return folder_location_exists * file_location_exists


def num_files(folder_location):
    # define our return variable, this one counts the number of files in a folder, so it is called quantity, start at
    # one to prevent counting errors; done by trial and error, I was always one file short on my quantity value
    quantity = 1
    # this will use the .iterdir() function of path to list the subdirectories of the folder_location path
    for path in Path(folder_location).iterdir():
        # if the subdirectory is a file then increment quantity
        if path.is_file():
            quantity += 1

    return quantity


def upload_folder(folder_location: str, file_name: str, database="NCID_local_DB", host="127.0.0.1"):

    # Check that user input was correct

    """
    if not error_check(folder_location, file_name):
        print("This folder or file path does not exist. Make sure the path was typed correctly.\n")
        return
    """

    # Get the number of files in the folder path given
    quantity = num_files(folder_location)

    for x in range(1, quantity):

        # Should have NCID variables fed to the call here from NCID API

        # This is a call class creation in order to ensure that each file is uploaded as a unique call
        call = Call()

        # This for loop makes sure that when a recording is input it correctly uses the file name
        # This is only necessary for our test audio files as when the ESP32 audio recording subsystem is integrated
        # it will name the audio recordings in a more easily manipulated manner EG: "Recording1" instead of
        # "Recording_000##" which will allows file_name = "Recording", quantity = # of recordings which will be
        # appended to file_name ("Recording").
        #
        # The for loop increments through the files in the folder path given in the function call then check the
        # quantity within the if statement below to ensure that the file number is properly appended. Then the recording
        # length is retrieved by the mutagen audio length functions included obtain_call_length. After that the file is
        # uploaded to the database with its original_length_of_call field updated to the length of the recording before
        # being processed and new_call_length updated to the silence removed length of the file.
        # This must be implemented in such a round-a-bout way because the OS functions built into python does not
        # include a way to access recording length of an audio file.

        # TODO: reimplement this function in 404 when audio recording subsystem is integrated to match naming scheme

        if x < 10:
            # Get creation date of file
            call.date_of_call = ocl.creation_date(folder_location, file_name + "00" + str(x) + ".wav")
            # Set unmodified file length of call
            call.original_length_of_call = ocl.obtain_length_of_call(folder_location + "/" + file_name + "00" + str(x))
            # Remove silence
            remove_silence(folder_location, "/" + file_name + "00" + str(x))
            # Set the length of call with silence removed
            call.new_length_of_call = ocl.obtain_length_of_call(folder_location + "_processed" + "/" + file_name + "00"
                                                                + str(x) + "_processed")
            # Set file name as specified by user
            call.file_name = file_name + str(x)
            # Upload the recording to the database
            call.upload_recording(folder_location + "_processed" + "/" + file_name + "00" + str(x) + "_processed.wav",
                                  database, host)

            # print("length of call: ", call.original_length_of_call)
            print("file name: ", call.file_name, "\n")

        elif 9 < x < 100:
            # Get creation date of file
            call.date_of_call = ocl.creation_date(folder_location, file_name + "0" + str(x) + ".wav")
            # Set unmodified file length of call
            call.original_length_of_call = ocl.obtain_length_of_call(folder_location + "/" + file_name + "0" + str(x))
            # Remove silence
            remove_silence(folder_location, "/" + file_name + "0" + str(x))
            # Set the length of call with silence removed
            call.new_length_of_call = ocl.obtain_length_of_call(folder_location + "_processed" + "/" + file_name + "0"
                                                                + str(x) + "_processed")
            # Set file name as specified by user
            call.file_name = file_name + str(x)
            # Upload the recording to the database
            call.upload_recording(folder_location + "_processed" + "/" + file_name + "0" + str(x) + "_processed.wav",
                                  database, host)

            # print("length of call: ", call.original_length_of_call)
            print("file name: ", call.file_name, "\n")

        else:
            # Get creation date of file
            call.date_of_call = ocl.creation_date(folder_location, file_name + str(x) + ".wav")
            # Set unmodified file length of call
            call.original_length_of_call = ocl.obtain_length_of_call(folder_location + "/" + file_name + str(x))
            # Remove silence
            remove_silence(folder_location, "/" + file_name + str(x))
            # Set the length of call with silence removed
            call.new_length_of_call = ocl.obtain_length_of_call(folder_location + "_processed" + "/" + file_name
                                                                + str(x) + "_processed")
            # Set file name as specified by user
            call.file_name = file_name + str(x)
            # Upload the recording to the database
            call.upload_recording(folder_location + "_processed" + "/" + file_name + str(x) + "_processed.wav",
                                  database, host)

        # print("length of call: ", call.original_length_of_call)
        print("file name: ", call.file_name, "\n")
