# from typing import List, Optional
from Scott.ECEN404Project.NCID_Database.services.calls import Call
from pathlib import Path
from playsound import playsound
# from data.incoming_callers import IncomingCaller
# from data.receiving_callers import ReceivingCaller
# from data.recordings import Recording
import services.upload_data as ud
from services.switchlang import switch
import services.state as state


def run():

    # This function starts the terminal UI. By using the run command in another function you
    # will allow the end user to navigate through the functions implemented into the show_commands() function

    print(' ****************** Welcome ****************** ')
    print(' *** Please log in using your NCID account *** ')
    print('')
    print('')
    print('')
    log_into_account()


def show_commands():

    # this print statement will be displayed to the end user when testing in the terminal
    # it lists all the options that the user can select
    # this will be implemented into NCID for ECEN404

    print('What action would you like to take:')
    # print('List callers *404 implementation*')
    print('list recent [c]alls')
    print('[u]pload')
    print('[d]elete')
    print('do[w]nload recording from database')
    print('[l]isten to recording')
    print('e[x]it app')
    print()

    # This switch statement takes a user input and uses it to execute the command described by the above print
    # statements. Additional statements can be added by specifying a unique alphanumeric input that corresponds to a
    # desired function

    # switch statement to select the correct function corresponding to user input
    while True:
        action = get_action()

        with switch(action) as s:
            s.case('c', view_calls)
            s.case('u', upload)
            s.case('l', listen_to_recording)
            s.case('d', delete_recording)
            s.case('w', download_recording)
            s.case(['x', 'bye', 'exit', 'exit()'], exit_app)
            s.case('', lambda: None)
            s.default(unknown_command)

        if action:
            print()

        if s.result == 'change_mode':
            return


def log_into_account():

    # This function will be implemented during ECEN404
    # Currently it only displays the LOGIN text and executes the show_commands() function, this ensures that when a user
    # logs in (in the future) they will need to verify that they have an NCID account before being able to access the
    # implemented commands

    print(' ****************** LOGIN ****************** ')

    # TODO: Get email
    # TODO: Find account in DB, set as logged in.

    # TODO: Make the show commands in an if statement checking if the user has logged in

    show_commands()  # this is done because we do not want someone to see the commands before logging in

    # print(" -------- NOT IMPLEMENTED (404 NCID integration) -------- ")


def view_users():

    # This function requires integration with the NCID program and will be completed in ECEN404
    # The intended functionality once implemented will allow the end user to view all unique users on this machine
    # where a user is someone with a unique voice signature

    print(' ****************** USERS ****************** ')

    # TODO: Require an account  <- wait until NCID integration
    # TODO: Get info about users who have accepted calls

    # print(" -------- NOT IMPLEMENTED (404 NCID integration) -------- ")


def list_user_details(supress_header=False):

    # This function is to be used in conjunction with view_users(), it will allow an end user to view the detailed
    # list of calls a unique voice signature has been involved in. This is useful for when an elderly person is in a
    # shared family household and the end user may not want the call log cluttered with calls that have a much
    # smaller likelihood of being a scam as they were made by a younger family member or the user themselves.

    if not supress_header:
        print(' ******************     User details     ****************** ')

    # TODO: Require an account with NCID
    # TODO: Will require voice signature matching for end user
    # TODO: Used as a function to expand a users information, including number of accepted incoming calls, as well as
    #       when those calls were accepted, the call spam confidence, and whether the incoming caller matches a known
    #       voice signature; requires a voice signature from ML subsystem to be implemented

    # print(" -------- NOT IMPLEMENTED (404 NCID integration) -------- ")


def view_calls():
    print(' ****************** YOUR CALLS ****************** ')

    # this function prints out information about all calls in the database, additional information can be added
    # by modifying the print statement in the for loop to include the information that you want

    query = Call.objects()
    calls_query = list(query)

    # return calls_query

    for c in calls_query:
        print(f'  *** FILE NAME: {c.file_name} *** \n '
              f' * DATE RECORDED: {c.date_of_call} \n '
              f' * DATE UPLOADED: {c.date_of_upload} \n '
              f' * ORIGINAL LENGTH: {c.original_length_of_call} \n '
              f' * STORED LENGTH: {c.new_length_of_call} \n '
              f'----------------------------------------')

    # show_commands() called to take user back to main menu
    show_commands()


def download_recording():
    # start collecting info from user
    boolean_existing_dir = input(r"Do you have an existing directory you want to download the file to? (y, n): ")
    print("\n")
    # make sure folder_path is outside of if statements, so it can be used later
    folder_path = ""
    if boolean_existing_dir.lower() == 'y':
        folder_path = input(r"Input the path to the folder using / or \, start at the highest directory "
                            r"(C:, D:, etc.): ")
        print("\n")
    elif boolean_existing_dir.lower() == 'n':
        folder_path = input(r"Input the path to the folder you wish to create using / or \, start at the highest "
                            r"directory (C:, D:, etc.): ")
        print("\n")
        # even though the user input that the path doesn't exist check before creating it
        if not Path(folder_path).exists():
            Path(folder_path).mkdir()
        else:
            continue_program = input("This file path already exists, do you wish to continue? (y, n): ")
            if continue_program.lower() != 'y':
                return
    else:
        print("Your input was invalid, please try again\n")
        # if the input is invalid make them input a valid string
        show_commands()

    # get the name of the recording as stored in the database
    name = input("Input the name of the recording: ")
    # retrieve the recording to the file specified
    ud.retrieve_recording(folder_path, name)
    print("Your file has been downloaded to: " + folder_path + "/" + name)
    # take user back to commands
    show_commands()


def listen_to_recording():

    # This code will be fully implemented in ECEN404, its functionality will allow the playback of messages using the
    # ESP32
    # The ESP32 recording subsystem is independent of the database code for ECEN403 but I did not want to forget about
    # its implementation next semester

    # start collecting info from user
    folder_path = input(r"Input the path to the folder using / or \, start at the highest directory "
                        r"(C:, D:, etc.): ")
    """
    removed because of implementation of retrieve file
    boolean_existing_dir = input(r"Do you have an existing directory you want to download the file to? (y, n): ")
    print("\n")
    # make sure folder_path is outside of if statements, so it can be used later
    folder_path = ""
    if boolean_existing_dir.lower() == 'y':
        folder_path = input(r"Input the path to the folder using / or , start at the highest directory "
                            r"(C:, D:, etc.): ")
        print("\n")
    elif boolean_existing_dir.lower() == 'n':
        folder_path = input(r"Input the path to the folder you wish to create using / or , start at the highest "
                            r"directory (C:, D:, etc.): ")
        print("\n")
        # even though the user input that the path doesn't exist check before creating it
        if not Path(folder_path).exists():
            Path(folder_path).mkdir()
        else:
            continue_program = input("This file path already exists, do you wish to continue? (y, n): ")
            if continue_program.lower() != 'y':
                return
    else:
        print("Your input was invalid, please try again\n")
        # if the input is invalid make them input a valid string
        show_commands()
    """

    # get the name of the recording as stored in the database
    name = input("Input the name of the recording: ")
    # perform error checking on file location to ensure a file is there
    if not ud.error_check_file(folder_path, name):
        print("This folder or file path does not exist. Make sure the path was typed correctly, and that the file has"
              " been downloaded from the database.\n")
        show_commands()
    # retrieve the recording to the file specified
    # ud.retrieve_recording(folder_path, name)
    # play the audio recording
    playsound(folder_path + "/" + name + ".wav")

    show_commands()


def upload():
    # user input actions
    print('What action would you like to take:')
    print('upload [f]older')
    print('upload f[i]le')
    print('[r]eturn to previous page')
    print('e[x]it app')
    print('[?] Help (this info)')
    print()

    # switch statement to select the correct function corresponding to user input
    while True:
        action = get_action()

        with switch(action) as s:
            s.case('f', upload_folder)
            s.case('i', upload_file)
            s.case('r', show_commands)
            s.case(['x', 'bye', 'exit', 'exit()'], exit_app)
            s.case('', lambda: None)
            s.default(unknown_command)

        if action:
            print()

        if s.result == 'change_mode':
            return


def upload_folder():
    # User instruction and input
    print(r"Please input the path to the folder containing your audio (starting at the highest directory; C:, D:, etc.)"
          r". use / or \ to separate your directories")
    folder_path = input(r"Input the path to the folder: ")
    print("\n")
    print("Now please input the file name; e.g. Recording1 would be input as Recording, Speaker_0000_00000 would "
          "be input as Speaker_0000_00.\nMake sure to check your file naming convention before inputting the name.")
    file_extension = input("Input the file name: ")
    print("\n")

    # call upload folder from upload_data file
    ud.upload_folder(folder_path, file_extension)

    # make sure to re-call show_commands() to ensure the user can still use the program
    show_commands()


def upload_file():

    # this function takes a users input as the file path to an audio recording, this is only used to manually upload
    # a given audio file to the database, for 404 this will be implemented as an automatic function call when the
    # audio recording of the call is complete. The functionality of this will still allow for debugging by users if they
    # choose to use this NCID module

    # get user input
    file_path = input("Input the path to the file (using / to separate file paths): ")
    file_name = input("Input file name: ")
    # define the call class
    call = Call()
    # notify user
    print("Starting folder upload...")
    # upload the file
    call.upload_recording(file_path, file_name)

    show_commands()


def delete_recording():
    # user input actions
    print('What action would you like to take:')
    print('delete file from [d]atabase')
    print('delete file from [l]ocal storage')
    print('[r]eturn to previous page')
    print('e[x]it app')
    print('[?] Help (this info)')
    print()

    # switch statement to select the correct function corresponding to user input
    while True:
        action = get_action()

        with switch(action) as s:
            s.case('d', delete_database)
            s.case('l', delete_local)
            s.case('r', show_commands)
            s.case(['x', 'bye', 'exit', 'exit()'], exit_app)
            s.case('', lambda: None)
            s.default(unknown_command)

        if action:
            print()

        if s.result == 'change_mode':
            return


def delete_database():
    # have user give database file name; can be checked by user using call_details
    name = input("Type the name of the file you want to remove: ")
    # remove the file
    ud.delete_database_recordings(name)

    show_commands()


def delete_local():
    # have the user give the path to the file that is desired
    folder_location = input("Type the path to the folder containing the file you want to be deleted: ")
    name = input("Type the name of the file you want to remove: ")
    # error check file path

    if not ud.error_check_file(folder_location, name):
        print("This folder or file path does not exist. Make sure the path was typed correctly.\n")
        show_commands()

    # remove the file
    ud.delete_local_recordings(folder_location, name)

    show_commands()


def exit_app():

    # exit commands UI and end the program

    print()
    print('bye')
    raise KeyboardInterrupt()


def get_action():

    # takes input text, is most often used in switch case statements

    text = '> '
    if state.active_account:
        text = f'{state.active_account.name}> '

    action = input(text)
    return action.strip().lower()


def unknown_command():

    # lets the user know that the command was not processed

    print("Sorry we didn't understand that command.")


def success_msg(text):

    # prints success message 'text'

    print(text)


def error_msg(text):

    # prints error message 'text'

    print(text)
