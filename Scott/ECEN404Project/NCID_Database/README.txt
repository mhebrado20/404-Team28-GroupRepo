README

## Documentation on database code written in python, this README is the most up to date version of what each function does ##

# Calls.py #
Calls.py contains the class in which all other classes will be nested. This class will then be the object queried by the database for important information 
that the end user may wish to access. I chose to contain all other classes within this nested class as otherwise it would be difficult to follow where 
information is being stored. This allows for easier identification and modification of the code by anyone who may wish to do so in the future. As this is 
an open source project I aimed to be mindful of future coders during the creation of classes.

Within calls.py the class Call() contains:
Variables:
    Date_of_call: the date the call was recorded
    Date_of_upload: the date the call was uploaded to the database
    Original_length_of_call: the length of the call before silence was removed
    New_length_of_call: the length of the call after silence is removed
    Call_recording: the embedded document that contains the recording
    File_name: the name of the file as specified by the user

Functions:
Upload_recording: this function takes a folder path and file name and uploads the document at that location to the database



# Incoming_callers.py #
This class will contain the voice signature of the incoming caller and will allow for easy matching of voice signatures, and look up of calls containing the 
voice signature of a specific person who may have perpetrated a scam.



# Mongoengine_setup.py #
This file contains setup information for the database. This was implemented as a function so that in the future should the database need to be configured 
to a different location it can be called with the new host IP and name of the database.


# Receiving_callers.py #
This class will contain the voice signature of the receiving caller and will allow for easy matching of voice signatures, 
and look up of calls containing the voice signature of a specific person who may have been victim to a scam.


# Audio_duration #
Audio_duration takes one argument, length, and converts that given length to a time format that is easily read. This is accomplished by floor dividing the given 
time by 3600 and assigning that to hours, then dividing the remainder of that by 60 and assigning that to minutes, and finally keeping the remainder of that 
division as the seconds. This function was necessary because when the length of an audio file is found it is given in seconds, which is not intuitive for the end 
user to read.


# Creation_date #
Creation_date takes two arguments, the folder path of the file you wish to find the creation date of, and the name of the file you wish to find the creation date of. 
Using the python os functions wrapped in the pathlib library this function takes the folder path and file name and finds a files creation date in seconds since the 
unix epoch date given as January 1st, 1970 at 00:00:0000 UTC. The function then computes the date of the creation from the seconds given and converts it into a time 
in the Central Standard Time Zone so as to give an accurate creation date for our time zone. The use of pathlib also ensures that no matter the operating system of 
the end user the proper path will be returned to the function.


# Obtain_length_of_call #
This function takes one argument, file location, and obtains the length of the call in seconds before returning the length in a readable format by calling the 
audio_duration function.


# Remove_silence.py #
This file contains a single function called remove_silence. This function takes two arguments, folder path that contains the required file where silence will be removed, 
and file name. The function will first read the file in the location given into a variable. Then the file will be segmented into one second portions of audio that will 
be analyzed for their ‘energy’. Energy in this context is a measure of the amount of speaking in the clip and helps determine whether the section of audio is silent or 
contains speech that may be important. After determining the energy of every one second clip the clips with energy below the threshold will be removed before concatenating 
the one second segments together again. After the segments are concatenated the function then checks if there is a folder for the processed clips to be uploaded to. If 
there is a folder containing the proper naming convention then the function will upload the processed clips to the folder using the naming convention provided by the 
user. If there is not a folder for the processed clips to be sent to then a folder is created, using the name provided by the user, in the same file location as the 
previous folder. Then the processed clips are uploaded to that folder. This process ensures that the original audio is not deleted until we are ready to do so.


# Upload_data.py #
Upload_data.py contains seven functions related to data manipulation and error checking within the database.


# Retrieve_recording #
Retrieve_recording takes four arguments, file location, file name in database, database name, database host ip. This function has been implemented in this way to streamline 
data retrieval should the project move in the direction of a cloud database. The two variables database name, and database host ip are not required and default to the local 
database. This function returns the first entry in the database that matches the name given in the function call to the location given by file location in the function call.


# Delete_local_recording #
Delete_local_recording takes two arguments, folder path, and file name. Given these two variables the function will find the file specified, error check the user input to 
make sure that the file exists, warn the user that they are deleting a file and prompt the user for confirmation, and then delete the local file.


# Delete_database_recording #
Delete_database_recording takes one argument, file name in database. Given the file name in the database this function will find the file name, error check the user 
input against the database to ensure the file exists, warn the user they are deleting a file from the database and prompt the user for confirmation, and then delete the 
file from the database.


# Error_check #
This is the first of two error checking functions, this function takes two arguments, folder path, and file name. This function was implemented to error check a wide variety 
of naming conventions from our dataset. The dataset had many different naming conventions for recordings and as such the error checking function was designed around checking 
the greatest number of these folders in our dataset to ensure that files existed within them. Given a folder path and file name this function will check to make sure that a 
file exists within the given folder path by using the python os functionality wrapped in the pathlib library. This ensures that you are not trying to upload data from an empty 
folder, and allows for the user to be prompted for a different folder directory without exiting the Demo UI used for testing. The use of pathlib ensures that these os path 
checking functions will be compatible across all platforms despite changes in path naming conventions. If the file path does not exist then the function returns a boolean false.


# Error_check_file #
This error checking function was designed to check if a single file existed, this function will be the error checking function of choice during integration as we will have 
full control over the naming conventions of our files and therefore will not need a specialized error checking function as explained above. The use of pathlib ensures that these 
os path checking functions will be compatible across all platforms despite changes in path naming conventions. This function takes two arguments, folder path, and file name. 
The function will then check the folder path to see if the file given exists within the path. If it does not then the function returns a boolean false.


# Num_files #
Num_files takes one argument, the folder path where the files are stored, and returns the number of files in that folder path. This function uses the pathlib library in order 
to ensure that the function will work regardless of the operating system of the end user.


# Upload_folder #
Upload_folder is the primary function of this file. Upload_folder takes three inputs, folder location, write location, and naming convention of the files (i.e. recording1, 
recording2, recording3, would be input as simply recording).Given the path to the folder you wish to upload, and the naming convention of the files within the folder, this 
function will automatically detect the number of files in the folder and upload them to the database while updating their Call() class variables. The function starts by error 
checking the users inputs to ensure that the folder actually contains files and exists. Then the function counts the number of files within the folder path given. After 
determining the number of files the function goes through a for loop where it executes the creation_date, obtain_length_of_call, silence removal, a second obtain_length_of_call, 
and updates the file name, before uploading the file to the database. Each of these functions updates a corresponding variable in the Call() class before the upload, ensuring 
that all the calls information is up to date.  The function then uploads a silence removed version of the audio named by the creation date of the file to the filesToTest 
directory for the machine learning to process.


# User_interface.py #
This file contains the functions that switch between a user's desired actions. When a user inputs a character the corresponding character in the switch case will be executed. 
Each of these functions executes a function from the database functions and classes section.


# Main.py #
Executes the mongoengine_setup() function and the user_interface, run(), functions which connect the database to the program and starts the Demo UI program so that the user 
can test the functionality of the database classes and functions.


# JSON_data_transfer.py #
This file contains the record_audio function and the ncid_message_print function. This file's objective is to capture the incoming serial data from the serial port and write 
the data to a file with a wav header so that it can be used by the machine learning algorithm, and then communicate with the NCID platform whether or not a file matches or 
does not match with a known voice in the database.


# record_audio #
This function takes a serial COM port, file location (to be written to), and the sample rate of the audio file and writes a wav file at the file location.


# ncid_msg_print #
ncid_msg_print takes a float value as its input and returns a message to the terminal with the confidence value of the machine learning algorithm. This function was originally 
intended to integrate with the NCID platform through a TCP IP port, however time constraints and bugs in other functions delayed that integration.


