from mutagen.wave import WAVE
from datetime import datetime, timedelta
import pathlib


def audio_duration(length: int):
    audio_hours = length // 3600  # calculate in hours
    length %= 3600
    audio_minutes = length // 60  # calculate in minutes
    length %= 60
    audio_seconds = length  # calculate in seconds

    return str(int(audio_hours)) + ':' + str(int(audio_minutes)) + ':' + str(int(audio_seconds))  # returns the duration


def creation_date(folder_location: str, file_name: str):
    # assign the creation time in milliseconds to create_time
    create_time = int(pathlib.Path(folder_location + '/' + file_name).stat().st_ctime)
    # compare to the epoch time to find the date it was uploaded
    converted_utc = datetime.fromtimestamp(create_time)
    # return value adjusted for timezone differences
    return converted_utc + timedelta(hours=8)


def obtain_length_of_call(file_location: str):
    audio = WAVE(file_location + ".wav")
    duration = int(audio.info.length)

    return audio_duration(duration)
