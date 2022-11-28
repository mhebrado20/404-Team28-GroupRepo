from mutagen.wave import WAVE
from datetime import timedelta
import pathlib
from datetime import datetime
from datetime import date


def audio_duration(length: int):
    audio_hours = length // 3600  # calculate in hours
    length %= 3600
    audio_minutes = length // 60  # calculate in minutes
    length %= 60
    audio_seconds = length  # calculate in seconds

    return str(int(audio_hours)) + ':' + str(int(audio_minutes)) + ':' + str(int(audio_seconds))  # returns the duration


def creation_date(folder_location: str, file_name: str):
    # assign the creation time in milliseconds to create_time
    create_time = int(pathlib.Path(folder_location + file_name).stat().st_ctime)
    # compare to the epoch time to find the date it was uploaded
    converted_utc = datetime.fromtimestamp(create_time)
    # return value adjusted for timezone differences
    return converted_utc


def creation_date2(folder_location: str, file_name: str):
    # assign the creation time in milliseconds to create_time
    create_time = int(pathlib.Path(folder_location + file_name).stat().st_ctime)
    # compare to the epoch time to find the date it was uploaded
    converted_utc = datetime.fromtimestamp(create_time)
    converted_utc = converted_utc.strftime("%y-%m-%d_%H.%M.%S")
    # return value adjusted for timezone differences
    return converted_utc


def obtain_length_of_call(file_location: str):
    # define the variable audio to be the wave file at file_location
    audio = WAVE(file_location + ".wav")
    # using the mutagen.wave function .info.length cast the float value to an int and assign it to duration
    duration = int(audio.info.length)
    # return the integer with the length of the call
    return audio_duration(duration)


def complete_time():
    time_now = datetime.now()
    current_time = time_now.strftime("%H.%M.%S")
    date_now = date.today()
    current_date = date_now.strftime("%y-%m-%d_")
    complete_time_return = current_date + current_time
    return complete_time_return
